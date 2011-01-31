#! /usr/bin/env perl


# remove the cookie and mapper files
unlink "mapper";
unlink "tmpcookies";

#
# Parse the arguments
#
$mailto = "";
$passwd = "";
$verbose = 0;
while ($arg = shift @ARGV)
{
    if ($arg eq "-v")
    {
        $verbose = 1;
    }
    elsif ($arg eq "-mailto")
    {
        $mailto = shift @ARGV;
        die "\nError: -mailto requires an argument.\n\n" if (!defined $mailto);
        die "\nError: -mailto expects an email address.\n\n" if (scalar($mailto =~ m/\@/) == 0);
    }
    elsif ($arg eq "-passwd")
    {
        $passwd = shift @ARGV;
        die "\nError: -passwd requires an argument.\n\n" if (!defined $passwd);
    }
    else
    {
        if ($arg eq "-h" or $arg eq "-help" or $arg eq "--help")
        {
            die "\nUsage: $0 [-mailto <recipient email>] [-passwd <list admin password>] [-v]\n\n";
        }
        else
        {
            die "\nError: unexpected argument: '$arg'\n\nUsage: $0 [-mailto <recipient email>] [-passwd <list admin password>]\n\n";
        }
    }
}

if ($passwd eq "")
{
    print "\n";
    print "Warning: cannot obtain current visit-developers email\n";
    print "roster without list admin password.  Skipping any\n";
    print "error checks which require examining this roster.\n";
    print "\n";
}

#
# Get the list of usernames who have made commits today
#
$date = `date +%Y%m%d`;
chomp($date);
@users = `svn log -r {${date}T0000}:{${date}T2359} http://portal.nersc.gov/svn/visit/ | egrep '^r[0-9]+ ' | cut -d\\\| -f2 | sort | uniq`;
foreach (@users) { s|^\s*(\S+)\s*\n$|$1|; }
if ($verbose)
{
    print "COMMITTERS=@users\n";
}

#
# Get the latest nersc username mapping file
#
@mapper = `svn cat http://portal.nersc.gov/svn/visit/trunk/src/svn_bin/nersc_username_to_email > mapper`;

#
# Map the user names to email addresses, looking for errors
#
@emails = ();
@missingusers = ();
foreach (@users)
{
   $email = `sh mapper $_`;
   chomp($email);
   if ($email eq 'visit-developers@ornl.gov')
   {
       push @missingusers, $_;
   }
   else
   {
       push @emails, $email;
   }
}

if ($verbose)
{
    print "COMMITTEREMAILS=@emails\n";
    print "MISSINGUSERS=@missingusers\n";
}

#
# Get the set of email address who are subscribed
# (or who are subscribed but disabled).
#
@notsubscribed = ();
@notenabled = ();
if ($passwd ne "")
{
    # authenticate as admin and save the cookie
    @auth = `wget -q -O- --keep-session-cookies --save-cookies=tmpcookies "https://email.ornl.gov/mailman/admin/visit-developers/members?adminpw=$passwd"`;
    # get the list using the authentication session cookie
    @html = `wget -q -O- --load-cookies=tmpcookies "https://email.ornl.gov/mailman/roster/visit-developers"`;
    # get just the members
    @members = grep(m|href="../options/visit-developers/|, @html);
    if ((scalar @members) < 1)
    {
        die "\nError: could not obtain the visit-devlopers roster.\nMaybe the password was incorrect?\n\n";
    }
    # split into enabled and disabled
    @enabled = grep(!m|\<em\>\(|,@members);
    @disabled = grep(m|\<em\>\(|,@members);
    # extract actual email addresses
    foreach (@enabled) { s|^.*\<a\s+href="[^"]+"\>\s*(\S+)\s+at\s+(\S+)\s*\</a>.*\n$|$1\@$2|g; }
    foreach (@disabled) { s|^.*\<a\s+href="[^"]+"\>\s*(\S+)\s+at\s+(\S+)\s*\</a>.*\n$|$1\@$2|g; }
    if ($verbose)
    {
        print "ENABLEDEMAILS=@enabled\n";
        print "DISABLEDEMAILS=@disabled\n";
    }

    #
    # Find any commiter emails without active subscriptions
    #
    foreach $email (@emails)
    {
        @match_enabled  = grep(m/^${email}$/, @enabled);
        @match_disabled = grep(m/^${email}$/, @disabled);
        if (scalar(@match_enabled) == 0)
        {
            if (scalar(@match_disabled) > 0)
            {
                push @notenabled, $email;
            }
            else
            {
                push @notsubscribed, $email;
            }
        }
    }
}

#
# Compose the error message.
#
$msg = "";
if (scalar(@missingusers) > 0)
{
    $msg .= "\n";
    $msg .= "The following committers are NOT IN NERSC_USERNAME_TO_EMAIL:\n";
    foreach (@missingusers) {$msg .= " $_\n";}
    $msg .= "\n";
}
if (scalar(@notsubscribed) > 0)
{
    $msg .= "\n";
    $msg .= "The following commiter email addresses are NOT SUBSCRIBED to visit-developers:\n";
    foreach (@notsubscribed) {$msg .= " $_\n";}
    $msg .= "\n";
}
if (scalar(@notenabled) > 0)
{
    $msg .= "\n";
    $msg .= "The following commiter email addresses subscribed to visit-developers BUT DISABLED:\n";
    foreach (@notenabled) {$msg .= " $_\n";}
    $msg .= "\n";
}

#
# If there's an error message, print or email it
#
if ($msg ne "")
{
    if ($mailto ne "")
    {
        open(MAIL, "|/usr/sbin/sendmail -t");
        print MAIL "To: $mailto\n";
        print MAIL "From: $mailto\n";
        print MAIL "Subject: Detected commiter/email problem\n";
        print MAIL "\n";
        print MAIL "From correlating with the commits made on $date, several problems\n";
        print MAIL "were discovered regarding developers and email subscriptions:\n";
        print MAIL $msg;
        close(MAIL);
        if ($verbose)
        {
            print "Errors were detected; details sent to $mailto.\n";
        }
    }
    else
    {
        print "Detected the following errors:\n";
        print "$msg";
    }
}
else
{
    if ($mailto ne "")
    {
        if ($verbose)
        {
            print "\nNo errors were detected; no email was sent to $mailto.\n";
        }
    }
    else
    {
        print "\nNo errors detected.\n\n";
    }
}

# cleanup: remove the cookie and mapper files
unlink "mapper";
unlink "tmpcookies";
