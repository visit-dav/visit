Software Reliability Engineering
================================

The IT world has many terms for services with similar sounding purposes such as
*Technical Support*, *Help Desk*, *Customer Support*, *Incident Response*,
*Service Desk*, *Incident Management*, *Cusomter Care*, etc.
`Specific meanings <https://www.atlassian.com/itsm/service-request-management/help-desk-vs-service-desk-vs-itsm>`_
of these terms are often driven by the software products IT companies sell to
support their
`management <https://www.bmc.com/blogs/help-desk-vs-service-desk-whats-difference/>`_.

In The VisIt_ project, *Customer Support* involves responding to inquiries about
the *use* of VisIt_ such as

  * How do I do a surface plot?
  * How do I compute the mass in a given material?
  * How do I get client/server to TACC working?

whereas *Incident Response* involves responding to incidents about *operational*
aspects of either

  * The VisIt_ software itself such as

    * A botched *managed* VisIt installation.
    * An update to host profiles to address site access changes.
    * A missing database reader plugin.

  * Or, the underlying computing infrastructure upon which VisIt_ depends such
    as

    * An incompatible grahics driver.
    * A downed file system or network.
    * A trip in the security environment.

In this document, we use the term **Customer Response** (an amalgamation of
*Customer Support* and *Incident Response*) to characterize the resources,
processes and activities the VisIt_ team employs to manage the work associated
with such inquiries.

As is typical of most DOE software projects, VisIt_ has no dedicated resources
for this purpose. Instead, developers themselves must also handle
**Customer Response** work. For many DOE software projects, managing this
activity effectively and efficiently is an essential part of maintaining the
overall quality of their software as well as the productivity of both developers
and users of the software alike.

Goals
-----

This document describes how the VisIt_ project manages **Customer Response**
activities. Some of the goals are...

  * To develop a practice of routine *housekeeping* quality improvements to the
    VisIt_ software and associated processes and artifacts impacting user and/or
    developer productivity.
  * To maintain a reputation for timely and quality response to customer needs.
  * To *load balance* the work of customer response in an equitable way across
    the development team.
  * To reduce customer response interruptions for the team as a whole.
  * To log, track and evolve a database of customer response activity and effort
    to help inform ongoing development plans and resource allocation.
  * To identify and document escalation paths for major incidents.
  * To aim for a four hour response time.

While many aspects of **Customer Response** are under the direct control of
VisIt_ developers, some are not and involve collaboration with other teams in
resolving. In most cases the extent of the VisIt_ team's involvement in
*operations* is confined primarly to the VisIt_ software itself; its
development, testing, release and deployment which includes installations the
VisIt_ team directly manages, hosted binary downloads for common platforms and
the tools and resources to build from sources. Operational issues impacting
VisIt_ but outside of this scope are typically delegated to other teams who
are responsible for the associated processes and resources.

In the IT world where companies like Google, Apple and Amazon have whole teams
dedicated to such activity, coverage is 24/7 and response time is measured in
*minutes*. For the VisIt_ project where the majority of funded development takes
place at Lawrence Livermore National Lab, coverage is during normal West Coast
*business* hours, 8am-12pm and 1-5pm (GMT-8, San Francisco time zone), Monday
through Friday excluding
`LLNL holidays <https://supplychain.llnl.gov/poattach/pdf/llnl_holidays.pdf>`_
and response time may be as much as four hours due to team members having to
multi-task among many responsibilities.

The Basic Process
-----------------

The work of customer response is allocated and rotated among developers in
one-week intervals. During a week, one developer is assigned as the **Primary**
customer response contact and a second developer is assigned as a **Backup**.

The role of the **Primary** is to *respond* within the response time goal, to
each inquiry. Ideally, all customer response activity during the week is handled
and *resolved* solely by the **Primary**. However, *escalations*, which we hope
are rare, will wind up engaging the **Backup** and may even engage other
developers. In addition, any *active* issues that remain unresolved at the end
of the week are formally *handed off* to the next **Primary**.

*Active* customer response issues will be logged and tracked in a separate GitHub,
`issues-only repository <https://github.com/visit-dav/live-customer-response/issues>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. Upon
resolution of *serious* incidents, the **Primary** will prepare a brief
*post-mortem* to inform a discussion of possible changes in practices at the
next project meeting.

Because customer response work tends to be interrupt driven, there is always
the chance that the **Primary** will have no *active* issues. At these times, the
**Primary** shall use their time to address general *housekeeping* or other
*low-hanging fruit* type work. In prticular, there shall be no expectation that
a developer serving as **Primary** can get any other work done beyond their
customer response and housekeeping obligations. In slow weeks, its conceivable
they can. But, there can be no implied assumption or expectation that this will
be the case.

A *schedule* of the primary and backup assignments going out several months is
periodically negotiated by the team and posted in the form of a shared Outlook
calendar. Primary and backup responsibilities are *rotated* so as to balance the
load among team members.

The preceding paragraphs describe VisIt_'s **Customer Response** processes at a
basic level and in the ideal. Aspects of this process were inspired in part by
`Google's Site Reliability Engineering book <https://landing.google.com/sre/sre-book/toc/>`_.
Nonetheless, several terms here (those in *italics*) require elaboration. In
addition, there are also many practical matters which can serve to complicate
the basic process. These details are addressed in the remaining sections.

Roles
-----

The primary's role is to respond, within the response time goal, to each inquiry 
that occurs during that week including those that came in during the preceding
weekend/holiday. The primary's goal is to *wrap-up* all inquiries by the end of
their week.

The **Primary** has the sole responsibility for responding to inquiries and
deciding next steps. The **Backup** is called into action only by explicit
request of the **Primary**. The **Primary** may temporarily delegate his/her
responsibilities to the **Backup** or enlist the **Backup** for help as
part of an escalation. To the extent possible, temporary delegation from
**Primary** to **Backup** should be handled formally and by mutual agreement.
For these reasons, the **Backup** is asked to at least maintain awareness of the
issues the **Primary** is handling.

In the ideal, the **Primary** is able to handle all customer response activity
and no other developers are engaged. However, this situation can change for
significant *escalations* (see below). Thus, other developers should feel free
to ignore customer inquiries as well as redirect customers who may contact them
directly via email, phone or walk-in. It is a best practice to handle such
redirections with a formal hand-off confirming that the customer makes contact
with the **Primary**.

Response Time and Response vs. Resolution
-----------------------------------------
The response time goal of four hours was chosen to reflect the worst case
practicalities of team member's schedules and responsibilities. For example, if
the **Primary** has meetings just before and just after the lunch hour break,
there can easily be a four hour period of time where inquiries go unattended.
Typically, we anticipate response times to be far less than four hours and
certainly, when able, the **Primary** should respond as quickly as practical and
not use the four hour goal as an excuse to delay a prompt response.

Since a majority of funding for VisIt_ is from LLNL and since VisIt_ developers
are co-located with many of its LLNL users, certainly these users as well as
their direct collaborators have an expectation of rapid response times. For
these reasons, the VisIt_ project operates a telephone hotline and also
frequently handles walk-ins. As an aside, after a recent small test effort to
maintain a rapid response time, a noticeable up-tick in user email inquiries was
observed suggesting that a faster response time has the effect of encouraging
more user interactions.

It is also important to distinguish between *response* and *resolution* here.
A key goal in this process is to ensure that customer inquires do not go left
unresponded for a long time. However, *responding* to a customer inquiry does
not necessarily mean *resolving* it. Sometimes, the only response possible is to
acknowledge the customer's inquiry and let them know that the resources to
address it will be allocated as soon as practical. In many cases, an *immediate*
response to acknowledge even just the receipt of a customer's inquiry with no
progress towards actual resolution goes a long way towards creating the goodwill
necessary to negotiate a day or more of time to respond more fully.

*Resolution* of a **Customer Response** issue often involves one or more of the
following activities...

  * Answering a question or referring a user to documentation.
  * Determining if the user's issue is known (e.g. an issue ticket already exists).
  * Updating a known issue with new information from this user, perhaps
    reprioritizing the issue or putting the issue ticket back into the
    UN-reviewed state for further discussion at a VisIt_ project meeting.
  * Diagnosing the incident.
  * Developing a reproducer for developers.
  * Developing a work-around for users.
  * Identifying a *low-hanging fruit* type issue and subsequently engaging
    in the *housekeeping* work to resolve it.
  * Identifying and filing a new *product development* type issue ticket.

To emphasize the last bullet above, *resolution* of a **Customer Response**
issue does not always mean a customer's issue can be addressed to *satisfaction*
within the constraints of our the processes defined here. Sometimes, the most
that can be achieved is filing a highly informative issue ticket to be
prioritized, scheduled and ultimately resolved as part of normal VisIt_ product
development activities. Nonetheless, doing so does serve to *resolve* the
original **Customer Response** issue that initiated the work.

Scheduling and Load Balancing
-----------------------------

To balance the work load of **Customer Response**, the responsibilities of the
**Primary** and **Backup** are rotated, round-robin among team members. For
example, on a team of eight developers, each would serve as **Primary** only one
week in eight or 12.5% of their time. However, a number of factors complicate
this simple approach including percent-time assignments of team members,
alternate work schedules, working remotely, travel, vacations, trainings,
meetings, etc.

On the one hand, it might make some sense to have developers that are 100% time
on the project serve as **Primary** twice as often as developers that are only
50% time. When the mix of such assignments are relatively well balanced, this
approach might make some sense. Because of all the factors that effect
scheduling, the VisIt_ project has opted to manage scheduling by periodically
negotiating assignments 1-3 months into the future and capturing the assignments
on a shared Outlook calendar. Either **Primary** or **Backup** can make last
minute changes to the schedule by finding a willing replacement and informing
the rest of the team of the change.

Whenever possible, an experienced **Backup** will be paired with a less
experienced **Primary**.

Handoffs
--------

If an active **Customer Response** issue cannot be resolved within the week of
a **Primary**'s assignment, it gets handed off to the next week's **Primary**.
Such handoffs shall be managed formally with an email to the customer(s) and the
next week's **Primary** and **Backup** 

Active Customer Response Issues Repo
------------------------------------

*Active* customer response issues will be logged and tracked in a separate GitHub,
`issues-only repository <https://github.com/visit-dav/live-customer-response/issues>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. 
For each new inquiry, the primary will file an issue ticket and assign themselves.
When the incident is resolved, the associated issue is closed. The primary will
endeavor to capture all relevant information and communications in this issue.
The use of GitHub issues for this purpose has a number of advantages over other
options such as email.

A number of steps have been taken to integrate the ``visit-users@elist.ornl.gov``
email list with this repository.

  * A new GitHub user account was created to serve as a proxy for the
    ``visit-users`` email list with name ``markcmiller86-visit``.
  * Added two-factor authentication to the GitHub proxy account. 
  * The ``visit-users`` email list configuration was adjusted in minor ways...
    * Added ``noreply@github.com`` and ``notifications@github.com`` to
      auto-accepted senders filter.
    * Turned off list setting ``require_explicit_destination`` because GitHub
      notifications can be sent in BCC field.
    * Turned off setting ``bounce_unrecognized_goes_to_list_owner`` to avoid
      some funky bounces from Outlook.com.

The procedure is for the **Primary** to cut-n-paste the initial email on
``visit-users`` to a new GitHub issue and from then on handle all communication
through the GitHub *conversation* associated with that issue. Each comment there
winds up generating an email to ``visit-users``. In addition, any reply from a
``visit-users`` email list member to any of the GitHub generated emails will
result in a new comment added to the GitHub issues. However, any replies to the
*initial* email (which is not generated by GitHub) will not route to the GitHub
issue conversation. The more quickly the **Primary** creates the associated
GitHub issue in response to the *initial* email, the less likely this will occur. 
In addition, boilerplate guidance in an issue template will help to mitigate
this.

Housekeeping and Low-hanging Fruit
----------------------------------

Another role of the primary is to use any time not working active inquiries to
fix *low-hanging fruit* issues; either those the primary is currently managing
or those from the backlog. As a rule of thumb, low-hanging fruit is considered
to be anything that the primary believes is fixable within a half-day's
(4 hours) worth of effort. When there are many such tasks in the system to work
on, the primary is free to use his/her judgement to decide which s/he can most
productively address.


Dealing with a Common Misconception: Customer Response is an *Interruption*.
----------------------------------------------------------------------------
When faced with a long backlog of development tasks, team members can all too
easily perceive customer response work as an *interruption* to those tasks.
This is a common misconception; one which project managers must continualy work
to correct. Customer response is an important aspect to a successful product and
project on par with any other major develpment work. It is part of what is
involved in keeping the software working and useful tool in our customer's
workflows not only here at LLNL, likely VisIt_'s biggest customer, but wherever
in DOE/DOD and elsewhere in the world VisIt_ is used.

Indeed, there are several *advantages* in having developers involved with
customer response activities.

   * Learn how users think
   * Learn how users use the tool
   * Learn what problem users apply the tool too
   * Learn what is easy and what is hard for users to do with tool
   * Learn where there is weak documentation
   * Learn weake

You identify user interface issues. You experience the ways in which VisIt can be hard to use and identify possible improvements.
You make connections with other people in the organization (which might be more useful to some of the newer team members)
In debugging issues, you wind up learning more about how to use VisIt
You learn more about the kinds of problems and issues our users face and how they operate and this can help inform many of your future activities with respect to capability and performance requirements.

The practice of having software development staff *integrated* with *operations*
is more commonly referred to as *DevOps*. There is a pretty good
`video <https://youtu.be/XoXeHdN2Ayc>`_ that introduces these concepts.

Escalation
----------
When primary has challenges with any of the paths to resolution above, s/he
should feel free to engage other developers with help. But primary should first
enlist the backup. When developer expertise other than backup is needed, primary
should arrange mutually agreeable times to engage with other developers.

Customer response incidents may escalate for a variety of reasons. The 
technical expertise or authority required may be beyond the primary's abilities.
Other difficulties may arise in responding to a given customer's needs.
For issues that the primary does not know how to resolve, the backup should be
enlisted first. It may just be a quick response that the backup can answer. If
the backup cannot help, the primary should enlist other developers using the @
mention feature of the associated GitHub issue. However, where a primary is
responsible for maintaining a one-hour response time, other VisIt_ developers
so enlisted are free to either delay or even decline to respond (but nonetheless
inform the primary of this need) if their current schedules do not permit
timely response.

If the work required to resolve a customer response incident is either not
known or not believed to be a *low-hanging-fruit* type task, the primary should
search the issue system to see if this is a known issue and, if so, add
additional information to that known issue about this new customer response
incident (and perhaps remove the *reviewed* tag from the issue to cause the
issue to be re-reviewed at the next VisIt_ project meeting) or submit a *new*
issue to the main repository issues.

Special Considerations for SCF
------------------------------

Occasionally, incidents arise that may be handled only in the Secure Computing
Facility (SCF). This is not too common but does happen and it presents problems
for a geographically distributed team.

On the one hand, customers on SCF are accustomed to longer response times.
On the other hand, often work on the SCF is a high priority and requires
rapid response from a developer that is on site with access to SCF.

In many ways, an SCF-only incident is just a different form of *escalation*.

Our current plan is to handle this on a case-by-case basis. If neither the
primary nor backup are able to handle a customer response incident requiring
the SCF, the primary should

  * First determine the customer's required response time. It may be hours
    or it may be days. If it is days. Its conceivable the issue could be
    handled in the following week by a new primary/backup pair.
  * If customer indicates immediate response required, primary should inquire
    the whole team to arrange another developer who can handle it.

Relation to Programmatic Work
-----------------------------

Add notes from email about what developers learn from this work

However, a variety of factors complicate a *simple* round-robin
style load balance. These complications are discussed
in THAT SECTION.

Ideally, on a team of 8, each developer will serve as primary only one week out
of every two months. This leads to fair load by head-count but isn't weighted by
project assignments. From a project development perspective, it might be more
appropriate for a developer that is only 50% time on VisIt_ to serve as the
primary only half as often as a 100% time developer. On the other hand, since
a majority of VisIt_ developers divide their time across multiple projects, we
use 50% as the sort of *nominal* developer assignment. We will load balance
weeks of customer response work equally across all developers but occasionally,
those with more than 50% time on VisIt_ will do an extra week.





I think this approach has a number of drawbacks
 
Unless you have the schedule memorized, its hard to know who is responsible for which incoming emails/calls and maybe nudge them if they missed something.
Customer support calls often unfold over the course of several conversations spanning several days and it makes the most sense for the initial developer POC to carry it all the way through when that is practical.
A lot of stuff piles up F pm through M am and so that Monday am slot is often a biggie.
We all travel, are split between multiple projects and wind up having the occasional fire drills that is doesn't mesh well with adherence to such a regular, fine-grained schedule.
 
In addition and FWIW...I've been making an effort to try to respond to customer support emails/calls as quickly as possible and whenever they have come in for a few reasons...
I'd like to see us be as responsive as possible
My time is so divided among projects sometimes I feel I am most useful to the VisIt project by unburdening others of these interruptions.
Many of the calls come up off-hours and I happen to be at my email.
I actually enjoy it (most of the time)
 
 
OTOH, customer support work is interrupt driven and gets in the way of your programmatic work too.
 
I think it would be better to have a primary and backup that cover a period of time of length a week or two or maybe even a whole month and then have this responsibility rotate. Someone else steps up to primary, primary steps to backup and backup along with everyone else is there only to the extent primary or backup require any further assistance which we hope will not occur very often or ever. An activity to include in our weekly meetings is any important stuff that primary/back up are dealing with as well as planning calendar for next primary/backup switchover.
 


Represents a reallocation of resources ~10% housekeeping activity
Work always associated with an issue
two classes of issues product development and devops


Software Reliability Engineeering / Site Reliability Engineering (SRE) Link to the google doc about this from Kevin.

In a real sense, the adoption of this process represents an adjustment in the
VisIt_ project's resource allocations.

