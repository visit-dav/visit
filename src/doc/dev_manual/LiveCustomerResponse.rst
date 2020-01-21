Customer Response
=================

We deliberately use the term **Customer Response** as an amalgamation of the
terms **Customer Support** and **Incident Response** to represent the fact that
both activities are involved here. **Customer Support** involves responding to
inquiries about the *use* of VisIt_. *How do I do X? Y is not working for me,*
*can you help? I am having trouble getting VisIt_ to work in client/server*.
These are examples of typical **Customer Support** inquiries VisIt_ developers
are routinely enlisted to answer.

In some cases, user's inquiries reveal a deeper, systemic, recently occurring
*operational* issue which may involve either an issue with the VisIt_ software
itself (e.g. a botched VisIt_ installation, an incompatability in client/server
protocol, a missing database reader plugin, etc.) or an issue with underlying
computational infrastructure upon which VisIt_ depends (incompatible grahics
drivers, a downed file system or network, a security environment trigger).
These latter issues are typical of **Incident Response** services and require
rapid response in diagnosing and enlisting resources to correct.

As is typical of most DOE software project teams, VisIt_ developers wind up
having to serve in both the roles of **Customer Support** and
**Incident Response**. So, we call this activity **Customer Response**.
While some aspects of **Customer Response** are under the direct control of
VisIt_ developers, many aspects are not and involve collaborations with other
teams in resolving. In most cases the extent of the VisIt_ team's involvement in
*operations* is confined primarly to the VisIt_ software itself; its
development, testing, release and deployment which includes installations we
directly support on systems where we have accounts, hosted binary downloads for
common platforms and the tools to build from sources.

The practice of having software development staff *integrated* with *operations*
is more commonly referred to as *DevOps*. There is a pretty good
`video <https://youtu.be/XoXeHdN2Ayc>`_ that introduces these concepts.

This document explains our procedures for how VisIt_ software developers handle
active customer response incidents. Some of the goals of this process are...

  * To try to *load balance* the work of customer response in an equitable way
    across the development team.
  * To achieve a one-hour response time.
  * To reduce incident response interruptions for the team as a whole.
  * To identify and document escalation paths for major incidents.

Meeting the one-hour response time goal
---------------------------------------
This is a desirable but challenging goal. We agree as a team to try to meet it.
But, we recognize that our schedules might not always permit strict adherence to
this goal. So, as an absolute, last resort, *bare minimum*, we expect issues
occurring 8am-3pm to be responded to by COB the same day. Issues occurring after
3pm but prior to start of next business day shall be responded to by noon of
that next day.

Note that *responding* to a customer inquiry does not necessarily
mean *resolving* it. Sometimes, the only response possible is to acknowledge the
customer's inquiry and let them know we intend to allocate resources to address
it as soon as practical. In many cases, an immediate response to acknowledge
even just the receipt of a customer's inquiry with no progress towards
actual resolution goes a long way towards creating the goodwill necessary to
negotiate a day or more of time to respond more fully.

One Week Cycles
---------------

For the VisIt_ project, normal business hours are 8am-12pm and 1-5pm, Monday
through Friday excluding LLNL holidays. The work of *live* customer response is
allocated in one-week intervals. During a week, one developer is assigned
as the **Primary** customer response contact and a second developer is assigned
as a **Backup**. Live customer response incidents will be logged and tracked in
a separate GitHub repository within the
`visit-dav GitHub organization. <https://github.com/visit-dav>`_

A schedule, showing the primary and backup assignments going out several months
will be posted on a
`wiki page <https://github.com/visit-dav/live-customer-response/wiki/VisIt-Customer-Response-Primary-and-Backup-Schedule)>`_.
Ideally, on a team of 8, each developer will serve as primary only one week out
of every two months. This leads to fair load by head-count but isn't weighted by
project assignments. From a project development perspective, it would be more
appropriate for a developer that is only 50% time on VisIt_ to serve as the
primary only half as often as a 100% time developer. On the other hand, few
developers are 100% time on VisIt. The majority of VisIt_ developers
divide their time across multiple projects. Using 50% as the sort of *nominal*
VisIt_ allocation, we expect to load balance weeks of customer response work
equally across all developers but infrequently have those with more than 50%
time on VisIt_ doing an extra week.

In the short term, it will help to pair veteran backups with a *primary*
who may have less experience with live customer response.

The Primary's Role
------------------

The primary's role is to respond, within one hour's time, to each incident
that occurs during that week including those that came in during the preceding
weekend/holiday. Other developers who may be contacted directly by customers
should redirect such inquires to the primary. The primary's goal is to *wrap-up*
all incidents by the end of their week. If an issue cannot be wrapped-up, it
gets handed-off to next week's primary. Handoff's are managed formally with an
email to customer and the next week's primary and backup contact information.

For each new incident, the primary should file an issue ticket in the
`live-customer-response <https://github.com/visit-dav/live-customer-response/issues>`_
repository and assign themselves. When the incident is resolved, the associated
issue should be closed. Primary should endeavor to capture all relevant
information and communications involving the incident in this issue. For users
who do not have GitHub accounts, this will require the primary to do a modest
amount of cutting and pasting between email and GitHub issues.

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
  * Identifying and filing a *new* coding issue in the main source repository.

In particular, please take note that *resolution* does not always mean a
customer's incident is able to be addressed favorably within the constraints of
our customer response processes. Sometimes, the best that can be achieved is
the filing of a highly informative issue ticket to be prioritized, scheduled
and ultimately resolved as part of normal weekly development meetings.

Another role of the primary is to use any time not working active incidents to
fix *low-hanging fruit* issues; either those the primary is currently managing
or those left undone from previous incidents. For example, if an incident
reveals incorrect or missing documentation, the primary should work to fix such
issues within his/her week. As a rule of thumb, low-hanging fruit is considered
to be anything that primary believes is fixable within a half-day's (4 hours)
worth of effort. 

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
form an outline of a *post-mortem* for the incident to be reviewed at VisIt_
project meetings.

The Backup's Role
-----------------

Ideally, like the rest of the development team, the backup is never called into
action for customer response. However, there are cases where the backup may be
needed. For this reason, the backup is asked to at least track and maintain
knowledge and awareness of the issues the primary is supporting. A good way to
achieve this is to selectively *watch* the visit-live-incidents repository.
In cases where primary needs help in resolving an incident, the backup
shall serve as the first line of defense. In addition, the backup should be
ready to step up to the primary role in the (hopefully rare) occurrence that the
primary is unable to maintain the one-hour response time goal. To the extent
possible, the transition from backup to primary should be managed formally.
Otherwise, the backup should simply listen in but otherwise not engage.

Other Developer's Role
----------------------

In the ideal, the primary is able to handle all customer response activity and
no other developers are interrupted. Thus, other developers should feel free to
ignore customer inquiries and/or redirect customers who may contact them directly.
One option to reduce visit-user traffic is to switch to receiving a daily digest.
Other options might be email client side filters.

Escalation
----------

Customer response incidents may escalate for a variety of reasons. The 
technical expertise or authority required may be beyond the primary's abilities.
Other difficulties may arise in responding to a given customer's needs.
For issues that the primary does not know how to resolve, the backup should be
enlisted. It may just be a quick response that the backup can answer. If the
backup cannot help, the primary should enlist other developers using the @
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

Occasionally, incidents arise that may be handled only on the SCF. This is
not too common but does happen and it presents problems for a geographically
distributed team.

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

When developers are faced with many development tasks, its easy to think of
customer response work as an *interruption* to those tasks. However, customer
response *is* indeed a critical aspect of programmatic work. It is part of what
is involved in keeping VisIt_ a working and useful tool in our customer's
workflows not only here at LLNL, likely VisIt_'s biggest customer, but
wherever in DOE/DOD and elsewhere in the world VisIt_ is used. Often the issues
users at large encounter and report are relevant to the use of VisIt_ here at
LLNL.

To Do
-----

  * Decide when to switch to this new process
  * Where do we keep weekly "calendar"?
    * maybe a wiki page or issue on live issues repo?
  * Mark to get telephone forwarding procedure
