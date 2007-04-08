#! /bin/env cqperl

use CQPerlExt;

use strict;

my $user = "";
my $pass = "";
my $output = "";
my $rulename = "Default";
my $db = "";

my $validscore = "";
my $project = "";
my $date = "";
my $vertgt = "";
my $verres = "";
my $state = "";
my $owner = "";
my $id = "";

if ($#ARGV == -1) {
  print "\nThis script scores ClearQuest tickets (selected by filters)";
  print "\naccording to predefined scoring rules.\n";
  print "\nRequired command line arguments:\n";
  print "\t-u  <user>   : User login\n";
  print "\t-p  <pass>   : User password\n";
  print "\t-db <db>     : Database name\n";
  print "\n";
  print "Optional command line arguments:\n";
  print "\t-r  <rule>   : Scoring rule to use (otherwise \"Default\")\n";
  print "\t-d           : Additional debug info\n";
  print "\t-o  <file>   : Write tab-delimited file\n";
  print "\n";
  print "Query filter arguments:\n";
  print "\t-validscore yes|no : Restrict to tickets with/without a valid score\n";
  print "\t-project <proj>: Restrict to tickets with project of <project>\n";
  print "\t-date <date>   : Restrict to tickets moved out of the Submitted\n";
  print "\t                 state since <date>\n";
  print "\t-state <states>: Restrict to tickets in given states (SIAOPRVC)\n";
  print "\t-vertgt <ver>  : Restrict to tickets with fix-by-version of <ver>\n";
  print "\t-verres <ver>  : Restrict to tickets actually resolved for <ver>\n";
  print "\t-owner <owner> : Restrict to tickets with owner of <owner>\n";
  print "\t-id <id>       : Restrict to tickets with a (partial) id of <id>\n";
  print "\n";
  print "Examples (dropping the required -u, -p, and -db options):\n";
  print "\n";
  print "1) score.pl -date 2005-10-01\n";
  print "\n";
  print "   This lists all tickets moved out of the Submitted state since Oct 1, 2005.\n";
  print "\n";
  print "\n";
  print "2) score.pl -owner meredith -state AO -version 1.5\n";
  print "\n";
  print "   This lists all tickets assigned to meredith, in the Assigned or Open\n";
  print "   states, with a fix-by-version of 1.5.\n";
  print "\n";
  print "\n";
  print "3) score.pl -state AIP -validscore no\n";
  print "\n";
  print "   This lists all tickets in the Assigned, Investigated or Postponed\n";
  print "   states without a valid score..\n";
  print "\n";
  print "\n";
  exit(0);
}

my $debug = 0;

# Get command line options
while (@ARGV) {
    $_ = shift @ARGV;
    if ( m/^\-[hH]$/ ) {         # -h... or -H...
        &usage;
        exit(0);
    }
    elsif ( m/^-u$/ ) {
        $user = shift @ARGV;
    }
    elsif ( m/^-p$/ ) {
        $pass = shift @ARGV;
    }
    elsif ( m/^\-o$/ ) {
        $output = shift @ARGV;
    }
    elsif ( m/^\-r$/ ) {
        $rulename = shift @ARGV;
    }
    elsif ( m/^-db$/ ) {
        $db = shift @ARGV;
    }
    elsif ( m/^-d$/ ) {
        $debug = 1;
    }
    elsif ( m/^-validscore$/ ) {
        $validscore = shift @ARGV;
        if ($validscore ne "yes" and $validscore ne "no")
        {
            die "Argument to -validscore must be 'yes' or 'no'.\n";
        }
    }
    elsif ( m/^-project$/ ) {
        $project = shift @ARGV;
    }
    elsif ( m/^-date$/ ) {
        $date = shift @ARGV;
    }
    elsif ( m/^-state$/ ) {
        $state = shift @ARGV;
    }
    elsif ( m/^-vertgt$/ ) {
        $vertgt = shift @ARGV;
    }
    elsif ( m/^-verres$/ ) {
        $verres = shift @ARGV;
    }
    elsif ( m/^-owner$/ ) {
        $owner = shift @ARGV;
    }
    elsif ( m/^-id$/ ) {
        $id = shift @ARGV;
    }
    else {
        die "Unrecognized argument: '$_'\n";
    }
}

if ($debug) {
    open(DEBUG, ">-");
}
else {
    open(DEBUG, ">/dev/null");
}

if (length($user) == 0) {
    print "Must supply username (-u)\n";
    exit(-1);
}
if (length($pass) == 0) {
    print "Must supply password (-p)\n";
    exit(-2);
}

# Open the ClearQuest session
my $session = CQSession::Build();

# Login
$session->UserLogon($user, $pass, $db, "");

# Query for all records resolved on the given branch
my $scorequery = $session->BuildQuery("ScoringRule");
$scorequery->BuildField("Name");
$scorequery->BuildField("Description");
$scorequery->BuildField("DefaultCustFactor");
$scorequery->BuildField("LikelihoodPower");
$scorequery->BuildField("SeriousnessScale");
$scorequery->BuildField("UserImprovementScale");
$scorequery->BuildField("DeveloperImprovementScale");
$scorequery->BuildField("SeriousnessPower");
$scorequery->BuildField("UserImprovementPower");
$scorequery->BuildField("DeveloperImprovementPower");
$scorequery->BuildField("TaskCostScale0");
$scorequery->BuildField("TaskCostScale1");
$scorequery->BuildField("TaskCostScale2");
$scorequery->BuildField("TaskCostScale3");
$scorequery->BuildField("TaskCostScale4");
$scorequery->BuildField("TaskCostScale5");
$scorequery->BuildField("ToolGrowthMultiplier");
$scorequery->BuildField("CustomerFactorWeight");
$scorequery->BuildField("ImportanceFactorWeight");
$scorequery->BuildField("FinalAveragingPower");
$scorequery->BuildField("WeightByTaskCost");
$scorequery->BuildField("AddScoringBonus");

# Pick only the scoring rule needed
my $scorenameop = $scorequery->BuildFilterOperator($CQPerlExt::CQ_BOOL_OP_AND);
$scorenameop->BuildFilter("Name",
                          $CQPerlExt::CQ_COMP_OP_EQ,
                          [$rulename]) ;

# If they specified '*' as the owner, then don't add in a
# query on that string
my $scoringrule = $session->BuildResultSet($scorequery);
$scoringrule->Execute();

if (($scoringrule->MoveNext()) != 1) {
    die "Couldn't find scoring rule by the name of \"$rulename\"\n";
}

## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------
## SCORING PARAMETERS
## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------

# Iterate over the list and set the resolved field
my $scoring_rule_name         = $scoringrule->GetColumnValue(1);
my $scoring_rule_description  = $scoringrule->GetColumnValue(2);
my $default_customer_factor   = $scoringrule->GetColumnValue(3);
my $likelihood_power          = $scoringrule->GetColumnValue(4);
my $seriousness_scale         = $scoringrule->GetColumnValue(5);
my $user_improve_scale        = $scoringrule->GetColumnValue(6);
my $devel_improve_scale       = $scoringrule->GetColumnValue(7);
my $seriousness_power         = $scoringrule->GetColumnValue(8);
my $user_improve_power        = $scoringrule->GetColumnValue(9);
my $devel_improve_power       = $scoringrule->GetColumnValue(10);
my @taskcost_scale            =($scoringrule->GetColumnValue(11),
                                $scoringrule->GetColumnValue(12),
                                $scoringrule->GetColumnValue(13),
                                $scoringrule->GetColumnValue(14),
                                $scoringrule->GetColumnValue(15),
                                $scoringrule->GetColumnValue(16));
my $toolgrowth_multiplier     = $scoringrule->GetColumnValue(17);
my $customer_factor_weight    = $scoringrule->GetColumnValue(18);
my $importance_factor_weight  = $scoringrule->GetColumnValue(19);
my $final_averaging_power     = $scoringrule->GetColumnValue(20);
my $weight_by_task_cost       = $scoringrule->GetColumnValue(21);
my $add_score_bonus_offset    = $scoringrule->GetColumnValue(22);


# Query for all records resolved on the given branch
my $querydef = $session->BuildQuery("Change_Request");
$querydef->BuildField("id");
$querydef->BuildField("State");
$querydef->BuildField("Headline");
$querydef->BuildField("Description");
$querydef->BuildField("Owner");
$querydef->BuildField("Likelihood");
$querydef->BuildField("Task_Cost");
$querydef->BuildField("Seriousness");
$querydef->BuildField("User_Improvement");
$querydef->BuildField("Developer_Improvement");
$querydef->BuildField("Tool_Growth");
$querydef->BuildField("External_Forces");
$querydef->BuildField("Customer_Opinion");
$querydef->BuildField("Scoring_Bonus");
$querydef->BuildField("Version_targeted");
$querydef->BuildField("Version_resolved");

my $operator = $querydef->BuildFilterOperator($CQPerlExt::CQ_BOOL_OP_AND);

if ($project ne "")
{
    $operator->BuildFilter("Project",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           [$project]);
}

if ($date ne "")
{
    $operator->BuildFilter("history.old_state",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           ["Submitted"]);
    $operator->BuildFilter("history.new_state",
                           $CQPerlExt::CQ_COMP_OP_NEQ,
                           ["Submitted"]);
    $operator->BuildFilter("history.action_timestamp",
                           $CQPerlExt::CQ_COMP_OP_GT,
                           [$date]);

}

if ($state ne "")
{
    my @states = ();
    my @selectedstates = split //, $state;
    foreach (@selectedstates)
    {
        if    (/^S$/i) {push @states, "Submitted";}
        elsif (/^I$/i) {push @states, "Investigated";}
        elsif (/^A$/i) {push @states, "Assigned";}
        elsif (/^O$/i) {push @states, "Opened";}
        elsif (/^R$/i) {push @states, "Resolved";}
        elsif (/^V$/i) {push @states, "Verified";}
        elsif (/^P$/i) {push @states, "Postponed";}
        elsif (/^D$/i) {push @states, "Duplicate";}
        elsif (/^C$/i) {push @states, "Closed";}
        else { die "Unexpected state '$_'"}
    }
    $operator->BuildFilter("State",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           \@states);
}

if ($owner ne "")
{
    $operator->BuildFilter("Owner",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           [$owner]);
}

if ($validscore eq "yes")
{
    $operator->BuildFilter("Likelihood",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("Task_Cost",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("Seriousness",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("User_Improvement",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("Developer_Improvement",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("Tool_Growth",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("External_Forces",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
    $operator->BuildFilter("Customer_Opinion",$CQPerlExt::CQ_COMP_OP_IS_NOT_NULL,[]);
}

if ($validscore eq "no")
{
    my $or_op = $operator->BuildFilterOperator($CQPerlExt::CQ_BOOL_OP_OR);
    $or_op->BuildFilter("Likelihood",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("Task_Cost",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("Seriousness",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("User_Improvement",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("Developer_Improvement",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("Tool_Growth",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("External_Forces",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
    $or_op->BuildFilter("Customer_Opinion",$CQPerlExt::CQ_COMP_OP_IS_NULL,[]);
}

if ($vertgt ne "")
{
    $operator->BuildFilter("Version_targeted",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           [$vertgt]);
}

if ($verres ne "")
{
    $operator->BuildFilter("Version_resolved",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           [$verres]);
}

if ($id ne "")
{
    $operator->BuildFilter("id",
                           $CQPerlExt::CQ_COMP_OP_LIKE,
                           [$id]);
}

if (0)
{
    $operator->BuildFilter("id",
                           $CQPerlExt::CQ_COMP_OP_EQ,
                           ["VisIt00005817",
                            "VisIt00005826",
                            "VisIt00005827",
                            "VisIt00005905",
                            "VisIt00005912",
                            "VisIt00005914",
                            "VisIt00006083",
                            "VisIt00006251",
                            "VisIt00006315",
                            "VisIt00006334",
                            "VisIt00006366",
                            "VisIt00006372",
                            "VisIt00006374",
                            "VisIt00006375",
                            "VisIt00006380",
                            "VisIt00006424",
                            "VisIt00006425",
                            "VisIt00006426",
                            "VisIt00006428",
                            "VisIt00006432",
                            "VisIt00006316",
                            "VisIt00006324",
                            "VisIt00006348",
                            "VisIt00006349",
                            "VisIt00006361",
                            "VisIt00006363",
                            "VisIt00006365",
                            "VisIt00006368",
                            "VisIt00006406",
                            "VisIt00006413",
                            "VisIt00006297",
                            "VisIt00006298",
                            "VisIt00006305",
                            "VisIt00006325",
                            "VisIt00006386",
                            "VisIt00006388",
                            "VisIt00006393",
                            "VisIt00006396",
                            "VisIt00006439",
                            "VisIt00006440",
                            "VisIt00006350",
                            "VisIt00006357",
                            "VisIt00006362",
                            "VisIt00006369",
                            "VisIt00006384",
                            "VisIt00006400",
                            "VisIt00006408",
                            "VisIt00006423",
                            "VisIt00006430",
                            "VisIt00006435",
                            "VisIt00006136",
                            "VisIt00006138",
                            "VisIt00006206",
                            "VisIt00006294",
                            "VisIt00006332",
                            "VisIt00006333",
                            "VisIt00006345",
                            "VisIt00006390",
                            "VisIt00006403",
                            "VisIt00006418"]);
}

my $resultset = $session->BuildResultSet($querydef);
$resultset->Execute();

## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------

if ($output ne "")
{
    open(OUTPUT, ">$output");
}

## ---------------------------------------------------------------------------
## ---------------------------------------------------------------------------

# Iterate over the list and set the resolved field
while (($resultset->MoveNext()) == 1) {

  my $ident  = $resultset->GetColumnValue(1);
  my $state  = $resultset->GetColumnValue(2);
  my $headl  = $resultset->GetColumnValue(3);
  my $descr  = $resultset->GetColumnValue(4);
  my $owner  = $resultset->GetColumnValue(5);

  my $li_str = $resultset->GetColumnValue(6);
  my $tc_str = $resultset->GetColumnValue(7);
  my $se_str = $resultset->GetColumnValue(8);
  my $iu_str = $resultset->GetColumnValue(9);
  my $id_str = $resultset->GetColumnValue(10);
  my $tg_str = $resultset->GetColumnValue(11);
  my $ef_str = $resultset->GetColumnValue(12);
  my $cp_str = $resultset->GetColumnValue(13);

  my $bonus  = $resultset->GetColumnValue(14);

  my $vertgt = $resultset->GetColumnValue(15);
  my $verres = $resultset->GetColumnValue(16);

  my $li          = -1;
  my $tc          = -1;
  my $se          = -1;
  my $iu          = -1;
  my $id          = -1;
  my $tg          = -1;
  my $ef          = -1;
  my $cp          = -1;
  my $final_score = -1;

  if ($li_str ne "" and
      $tc_str ne "" and
      $se_str ne "" and
      $iu_str ne "" and
      $id_str ne "" and
      $tg_str ne "" and
      $ef_str ne "" and
      $cp_str ne "")
  {
      $li = substr($li_str, 0,1);
      $tc = substr($tc_str, 0,1);
      $se = substr($se_str, 0,1);
      $iu = substr($iu_str, 0,1);
      $id = substr($id_str, 0,1);
      $tg = substr($tg_str, 0,1);
      $ef = substr($ef_str, 0,1);
      $cp = substr($cp_str, 0,1);

      print DEBUG "\n";
      print DEBUG "$ident $state $headl\n";
      print DEBUG "        li=$li tc=$tc se=$se iu=$iu id=$id tg=$tg ef=$ef cp=$cp\n";

      my $customer_factor = $default_customer_factor;
      if ($ef != 0 and $cp != 0) {
          my $ef_alpha = ($ef - 1.0)/4.0;
          my $cp_alpha = ($cp - 1.0)/4.0;
          print DEBUG "\t\t\t ef_alpha = $ef_alpha\n";
          print DEBUG "\t\t\t cp_alpha = $cp_alpha\n";

          my $cp_at_ef1 = 40.0 * $cp_alpha;
          my $cp_at_ef5 = 100 * ($cp_alpha ** (1./4.));

          print DEBUG "\t\t\t cp_at_ef1 = $cp_at_ef1\n";
          print DEBUG "\t\t\t cp_at_ef5 = $cp_at_ef5\n";

          $customer_factor = (1-$ef_alpha) * $cp_at_ef1  +  ($ef_alpha) * $cp_at_ef5;
      }
      print DEBUG "\t\t\t customer_factor = $customer_factor\n";

      my $new_final_averaging_power = $final_averaging_power;
      $new_final_averaging_power **= (1./3.) if ($cp == 1);
      $new_final_averaging_power **= (1./2.) if ($cp == 2);
      $new_final_averaging_power **= 2.0     if ($cp == 4);
      $new_final_averaging_power **= 3.0     if ($cp == 5);
      print DEBUG "\t\t\t final_averaging_power = $final_averaging_power\n";
      print DEBUG "\t\t\t new_final_averaging_power = $new_final_averaging_power\n";

      my $li_alpha = ($li - 1.0)/4.0;
      my $li_accelerated = $li_alpha ** $likelihood_power;

      print DEBUG "\t\t\t li_accelerated = $li_accelerated\n";

      my $se_final = 0;
      my $iu_final = 0;
      my $id_final = 0;

      if ($se > 0) {
          my $se_alpha = ($se - 1.0)/4.0;
          my $se_accelerated = $se_alpha ** $seriousness_power;
          $se_final = $se_accelerated * $seriousness_scale;
          print DEBUG "\t\t\t se_final = " . ($se_final *4+1*$seriousness_scale) . "\n";
      }

      if ($iu > 0) {
          my $iu_alpha = ($iu - 1.0)/4.0;
          my $iu_accelerated = $iu_alpha ** $user_improve_power;
          $iu_final = $iu_accelerated * $user_improve_scale;
          print DEBUG "\t\t\t iu_final = " . ($iu_final *4+1*$user_improve_scale) . "\n";
      }

      if ($id > 0) {
          my $id_alpha = ($id - 1.0)/4.0;
          my $id_accelerated = $id_alpha ** $devel_improve_power;
          $id_final = $id_accelerated * $devel_improve_scale;
          print DEBUG "\t\t\t id_final = " . ($id_final *4+1*$devel_improve_scale) . "\n";
      }


      my $consequence = max($se_final, $iu_final, $id_final);
      print DEBUG "\t\t\t consequence = $consequence\n";

      my $importance_at_li1 =  0 + 20 * $consequence;
      my $importance_at_li5 = 20 + 80 * $consequence;
      my $importance_factor = (1-$li_accelerated) * $importance_at_li1 + ($li_accelerated) * $importance_at_li5;

      print DEBUG "\t\t\t importance_factor = $importance_factor\n";

      my $tc_weight = $taskcost_scale[$tc];

      print DEBUG "\t\t\t tc_weight = $tc_weight\n";

      my $tg_factor = $toolgrowth_multiplier * $tg;

      my $bonus_factor = ($add_score_bonus_offset ? $bonus : 0);

      my $weight_sum = $importance_factor_weight + $customer_factor_weight;
      my $raw_score = ((($importance_factor ** $new_final_averaging_power) * $importance_factor_weight +
                        ($customer_factor   ** $new_final_averaging_power) * $customer_factor_weight   )
                       / $weight_sum) ** (1. / $new_final_averaging_power) + $tg_factor;

      print DEBUG "\t\t\t raw_score = $raw_score\n";

      if ($weight_by_task_cost) {
          $final_score = $tc_weight * $raw_score + $bonus_factor;
      } else {
          $final_score = $raw_score + $bonus_factor;
      }
      print DEBUG "\t\t\t final_Score = $final_score\n";
  }
  else
  {
      $li = substr($li_str, 0,1) if $li_str ne "";
      $tc = substr($tc_str, 0,1) if $tc_str ne "";
      $se = substr($se_str, 0,1) if $se_str ne "";
      $iu = substr($iu_str, 0,1) if $iu_str ne "";
      $id = substr($id_str, 0,1) if $id_str ne "";
      $tg = substr($tg_str, 0,1) if $tg_str ne "";
      $ef = substr($ef_str, 0,1) if $ef_str ne "";
      $cp = substr($cp_str, 0,1) if $cp_str ne "";
  }

  if (! $debug and $output eq "")
  {
      print sprintf("%6.5s %s %s %s\n",$final_score, $ident, $state, $headl);
  }

  if ($output ne "")
  {
      # get rid of tabs and ^M's in the text fields
      $headl =~ s/\t/\        /g;
      $headl =~ s/\r//g;
      $descr =~ s/\t/\        /g;
      $descr =~ s/\r//g;

      print OUTPUT "$final_score\t".
                   "$ident\t$state\t$headl\t$owner\t".
                   "$li\t$tc\t$se\t$iu\t$id\t$tg\t$ef\t$cp\t".
                   "$bonus\t$vertgt\t$verres\t".
                   "$descr\t".
                   "\n";
  }
}

if ($output ne "")
{
    close(OUTPUT);
}

close(DEBUG);

# Close up the session
CQSession::Unbuild($session);


## ---------------------------------------------------------------------------

sub max {
    my @values = @_;
    my $maxval = $values[0];
    foreach (@values)
    {
        if ($_ > $maxval)
        {
            $maxval = $_;
        }
    }
    return $maxval;
}
