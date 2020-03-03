Customer Response
=================

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
processes and activities the VisIt_ team employs to manage such inquiries.

As is typical of most DOE software projects, VisIt_ has no dedicated resources
for this purpose. Instead, developers themselves must also handle
**Customer Response** work. Managing this activity effectively is an essential
part of maintaining the overall quality software as well as the productivity of
both developers and users of the software alike.

Customer Response Process Goals
-------------------------------

This document describes the process for how the VisIt_ project manages
customer response activities. Some of the goals of this process are...

  * To try to *load balance* the work of customer response in an equitable way
    across the development team.
  * To aim for a four-hour response time goal. **Note**: *response* does not
    necessarily mean resolution (see below).
  * To reduce customer response interruptions for the team as a whole.
  * To log, track and evolve a database of customer response activity and effort.
  * To maintain a reputation for timely and quality response to customer needs.
  * To develop a practice of routine housekeeping fixes for
    low-hanging-fruit type issues impacting user and/or developer productivity.
  * To identify and document escalation paths for major incidents.

While many aspects of **Customer Response** are under the direct control of
VisIt_ developers, some are not and involve collaboration with other teams in
resolving. In most cases the extent of the VisIt_ team's involvement in
*operations* is confined primarly to the VisIt_ software itself; its
development, testing, release and deployment which includes installations the
VisIt_ team directly manages, hosted binary downloads for common platforms and
the tools to build from sources. Operational issues impacting VisIt_ but outside
of this scope are typically delegated to other teams.

In the IT world where companies like Google, Apple and Amazon have whole teams
dedicated to such activity, coverage is 24/7 and response time is measured in
*minutes*. For the VisIt_ project where the majority of funded development takes
place at Lawrence Livermore National Lab, coverage is during normal West Coast
*business* hours, 8am-12pm and 1-5pm (GMT-8, San Francisco time zone), Monday
through Friday excluding
`LLNL holidays <https://supplychain.llnl.gov/poattach/pdf/llnl_holidays.pdf>`_
and response time is realistically about four hours due to team members having
to multi-task among many responsibilities.

One Week Rotations
------------------
The work of customer response is assigned and rotated among developers in
one-week intervals. During a week, one developer is assigned as the **Primary**
customer response contact and a second developer is assigned as a **Backup**.

Live customer response inquiries will be logged and tracked in a separate GitHub
repository within the
`visit-dav GitHub organization. <https://github.com/visit-dav>`_

A schedule of the primary and backup assignments going out several months is
periodically negotiated by the team and posted in the form of a shared Outlook
calendar.

Primary and backup responsibilities are rotated so as to balance the load among
team members.

In the short term, it will help to pair an experienced backup with a primary who may
have less experience with live customer response.

The Primary's Role
------------------

The primary's role is to respond, within the response time goal, to each incident
that occurs during that week including those that came in during the preceding
weekend/holiday. Other developers who may be contacted directly by customers
should redirect such inquires to the primary. The primary's goal is to *wrap-up*
all incidents by the end of their week. If an issue cannot be wrapped-up, it
gets handed-off to next week's primary. Handoff's are managed formally with an
email to the customer and the next week's primary and backup contact information.

For each new incident, the primary will file an issue ticket in the
`live-customer-response <https://github.com/visit-dav/live-customer-response/issues>`_
repository and assign themselves. When the incident is resolved, the associated
issue is closed. The primary will endeavor to capture all relevant information
and communications involving the incident in this issue. For users who do not
have GitHub accounts, this may require the primary to do a modest amount of
cutting and pasting between email and GitHub issues. However, 

The goal of the primary is to reach a *resolution* of each incident. Resolution
often involves one or more of the following activities.

  * Answering a question or referring user to documentation.
  * Determining if user's issue is known (e.g. an issue ticket already exists).
  * Updating a known issue with additional information from this user, perhaps
    reprioritizing the issue or putting the issue ticket back into the
    UN-reviewed state for further discussion at a VisIt_ project meeting.
  * Diagnosing the incident.
  * Identifying and subsequently fixing any *low-hanging fruit*.
  * Developing a reproducer for developers.
  * Developing a work-around for users.
  * Identifying and filing a *new* issue ticket.

In particular, please take note that *resolution* does not always mean a
customer's incident can be addressed favorably within the constraints of
our customer response processes. Sometimes, the best that can be achieved is
the filing of a highly informative issue ticket to be prioritized, scheduled
and ultimately resolved as part of normal weekly development activities.

Another role of the primary is to use any time not working active inquiries to
fix *low-hanging fruit* issues; either those the primary is currently managing
or those from the backlog. As a rule of thumb, low-hanging fruit is considered
to be anything that the primary believes is fixable within a half-day's
(4 hours) worth of effort. When there are many such tasks in the system to work
on, the primary is free to use his/her judgement to decide which are most
productive to address.

When primary has challenges with any of the paths to resolution above, s/he
should feel free to engage other developers with help. But primary should first
enlist the backup. When developer expertise other than backup is needed, primary
should arrange mutually agreeable times to engage those other developers.

There shall be no expectation that a developer serving as primary can get any
other work done beyond their customer response obligations. In slow weeks, its
conceivable they can. But, there can be no implied assumption or expectation
that this will be the case. Furthermore, even in a slow week, the primary may
likely be able to find plenty of *low-hanging-fruit-type* tasks in the main
repository issues to tackle and fix.

During their communication's with the customer, the primary should avoid
reply-all-type communications.

For applicable incidents, the primary should prepare a set of bullets to
form an outline of a *post-mortem* for the incident to be reviewed at the next
VisIt_ project meeting.

The Backup's Role
-----------------

Ideally, like the rest of the development team, the backup is never called into
action for customer response. However, there are cases where the backup may be
needed. For this reason, the backup is asked to at least track and maintain
knowledge and awareness of the issues the primary is handling. A good way to
achieve this is to selectively *watch* the visit-live-incidents repository.
In cases where primary needs help in resolving an incident, the backup
shall serve as the first line of defense. In addition, the backup should be
ready to step up to the primary role in the (hopefully rare) occurrence that the
primary is unable to maintain the one-hour response time goal. To the extent
possible, the transition from backup to primary should be managed formally.
Otherwise, the backup should simply track but otherwise not engage in related
communications.

Other Developers' Role
----------------------

In the ideal, the primary is able to handle all customer response activity and
no other developers are interrupted. Thus, other developers should feel free to
ignore customer inquiries and/or redirect customers who may contact them directly.
One option to reduce communication traffic is either to switch to receiving a
daily digest during off weeks and/or UNwatch the 
`live-customer-response <https://github.com/visit-dav/live-customer-response/issues>`_
repository.

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

Meeting the one-hour response time goal
---------------------------------------
This is a desirable but also challenging goal. Certainly LLNL users as well as
their direct collaborators have an expectation of rapid response times. The
VisIt_ project is co-located with many its 

operates a telephone hotline as well as walk-ins in part to
support rapid response. As an aside, 
Following recent efforts to improve response times in the past year, there has
been a noticeable up-tick in utilization of 

the VisIt_ team has noticed
an increase in 

We agree as a team to try to meet it.
But, we recognize that our schedules might not always permit strict adherence to
this goal. So, as an absolute, last resort, *bare minimum*, we expect issues
occurring 8am-3pm to be responded to by COB the same day. Issues occurring after
3pm but prior to start of next business day shall be responded to by noon of
that next day.

WE REALLY SHOULD BE THIS RESPONSIVE FOR LLNL CUSTOMERS

Note that *responding* to a customer inquiry does not necessarily
mean *resolving* it. Sometimes, the only response possible is to acknowledge the
customer's inquiry and let them know we intend to allocate resources to address
it as soon as practical. In many cases, an immediate response to acknowledge
even just the receipt of a customer's inquiry with no progress towards
actual resolution goes a long way towards creating the goodwill necessary to
negotiate a day or more of time to respond more fully.

Escalation
----------

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

Complicating Factors
--------------------

A number of factors threaten

* SCF
* Part-time
* Travel
* AWS
* Experience level
* Direct interactions
* Hotline, walk-ins 

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
 
