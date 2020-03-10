Site Reliability Engineering (SRE)
==================================

In the VisIt_ project, members of the development team are frequently called
upon to respond to a variety of inquiries often originating directly from users.
Some of these may relate to the *use* of VisIt_ such as

  * How do I do a surface plot?
  * How do I compute the mass in a given material?
  * How do I get client/server to TACC working?

and some may relate to an *operational* aspect of either

  * The VisIt_ software itself such as

    * A botched *managed* VisIt installation.
    * An update to host profiles to address site access changes.
    * A missing database reader plugin.

  * Or, the underlying computing infrastructure upon which VisIt_ depends such
    as

    * An incompatible graphics driver.
    * A downed file system or network.
    * A trip in the security environment.

Typically such inquiries originate from users who are in the midst of trying
to use the VisIt_ software and are encountering some kind of difficulty.
However, for highly effective software projects, the work involved in handling
such inquiries does not end with simply diagnosing the user's problem(s) and
proposing solutions. Often the problems users encounter are suggestive of
deficiencies in either the software itself or its associated processes and
artifacts. Continually investing the time to identify appropriate corrections
and carry them out in response to such inquiries is a *best practice* similar
in many respects to Google's
`Site Reliability Engineering (SRE) <https://landing.google.com/sre/sre-book/toc/>`_
process sometimes
`also called <https://www.cio.com/article/3192531/why-you-need-a-systems-reliability-engineer.html>`_
*Systems Reliability Engineering* or *Services Reliability Engineering*.
Because our primary focus here is on *software*, it might be appealing to
call it *Software Reliability Engineering*. However that name already has a
specific meaning in the software industry and it is in no way what we mean by
SRE here.

For mature DOE software projects with long histories and many users, SRE activity
represents a wholly different brand of effort than conventional software product
development, planning and execution. Most DOE software projects, have no dedicated
SRE resources. The VisIt_ is no exception. Instead, developers themselves must also
handle SRE activities. Nonetheless, managing SRE work effectively and efficiently is
an essential part of maintaining the overall quality of the software as well as the
productivity of both developers and users of the software alike.

Goals
-----

This document describes how the VisIt_ project manages SRE activities.
Some of the goals of this process are...

  * To maintain a reputation for timely and quality response to customer needs.
  * To develop a practice of routine *housekeeping* quality improvements to the
    VisIt_ software and associated processes and artifacts impacting user and/or
    developer productivity.
  * To *load balance* SRE work in an equitable way across the development team.
  * To reduce SRE interruptions for the team as a whole.
  * To log, track and evolve a database of SRE activity and effort to help inform
     ongoing development plans and resource allocation.
  * To identify and document escalation paths for major incidents.
  * To aim for a four hour response time.

While many aspects of SRE are under the direct control of
VisIt_ developers, some are not and involve collaboration with other teams in
resolving. In most cases the extent of the VisIt_ team's involvement in the
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

SRE work is allocated and rotated among developers in
one-week intervals. During a week, one developer's `role <sre_roles>` is to
be the **Primary** SRE contact and a second developer's `role <sre_roles>` is to
be a **Backup**. Except for `escalations <sre_escalations>`, all other developers
are free of SRE responsibilities for that week.

The `role <sre_role>` of the **Primary** is to `respond <sre_response_vs_resolution>`
within the response time goal, to each inquiry. Ideally, all SRE
activity during the week is handled and `resolved <sre_response_vs_resolution>`
solely by the **Primary**. However, `escalations <sre_escalations>`, which we
hope are rare, will wind up engaging the **Backup** and may even engage other
developers. In addition, any `active issues <sre_active_issues>` that remain
unresolved at the end of the week are formally `handed off <sre_handoffs>` to
the next **Primary**.

`Active <sre_active_issues>` SRE issues will be logged and tracked in a separate GitHub,
`issues-only repository <https://github.com/visit-dav/live-customer-response/issues>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. Upon
resolution of *serious* incidents, the **Primary** will prepare a brief
*postmortem* to inform a discussion at the next project meeting of possible changes
in practices to avoid such major incidents.

.. danger::
   * Lets define serious and postmortem

Because SRE work tends to be interrupt driven, there is always
the chance that the **Primary** will have no *active* issues. At these *idle* times, the
**Primary** shall use their time to address general `housekeeping <sre_housekeeping>`
or other *low-hanging fruit* type work. In particular, there shall be no expectation
that a developer serving as **Primary** can get any other work done beyond their
active or idle SRE obligations. In slow weeks, its conceivable they can. But, there
can be no implied assumption or expectation that this will be the case.

A `schedule <sre_schedule>` of the **Primary** and **Backup** assignments going
out several months is periodically negotiated by the team and posted in the form
of a shared calendar. **Primary** and **Backup** responsibilities are rotated
so as to balance the load among team members.

The preceding paragraphs describe VisIt_'s SRE processes at a
basic level and in the ideal. Nonetheless, several terms here (those that are 
links or in *italics* in the paragraphs above) require elaboration. In addition,
there are also many practical matters which can serve to complicate the basic
process. These details are addressed in the remaining sections.

.. danger::
   * Did we introduce all the section headings

.. _sre_roles:

Roles
-----

The **Primary**'s role is to respond, within the response time goal, to each
inquiry that occurs during that week including those that come in during the
preceding weekend/holiday. The **Primary**'s goal is to *resolve* (see below)
all inquiries by the end of their week.

The **Primary** has the sole responsibility for responding to inquiries and
deciding next steps. The **Backup** is called into action only by explicit
request of the **Primary**. The **Primary** may temporarily delegate his/her
responsibilities to the **Backup** or enlist the **Backup** for help as
part of an escalation. To the extent possible, temporary delegation from
**Primary** to **Backup** should be handled formally and by mutual agreement.
For these reasons, the **Backup** is asked to at least maintain awareness of the
issues the **Primary** is handling.

Ideally, the **Primary** is able to handle all SRE activity
and no other developers are engaged. However, this situation can change for
significant `escalations <sre_escalations>`. Thus, other developers are free to
ignore customer inquiries as well as redirect customers who may contact them directly
via email, phone or walk-in. It is a best practice to handle such redirections
with a formal, three-way handoff confirming that the customer indeed makes
contact with the **Primary**.

.. _sre_active_issues:

Active Customer Response Issues Repo
------------------------------------

*Active* customer response issues will be logged and tracked in a separate GitHub,
`issues-only repository <https://github.com/visit-dav/live-customer-response/issues>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. 
For each new inquiry, the primary will file an issue ticket and assign themselves.
When the inquiry is resolved, the associated issue is closed. The primary will
endeavor to capture all relevant information and communications in this issue.
The use of GitHub issues for this purpose has a number of advantages over other
options such as email including better support for attachments. For this reason,
a number of steps were taken to integrate the ``visit-users@elist.ornl.gov`` email
list with this issues-only repository.

Upon receiving a *new* inquiry on the ``visit-users`` email list (or a telephone
hotline call or walk-in with the exception of those involving classified information),
the procedure is for the **Primary** to cut-n-paste the initial email to a new GitHub
`SRE issue <https://github.com/visit-dav/live-customer-response/issues>`_
and from then on handle all communication
through the *conversation* associated with that issue. Each comment there
will generate an email to ``visit-users``. In addition, any reply to any
GitHub generated emails will result in a new comment added to the GitHub issues
as though it came from GitHub user account ``markcmiller86-visit``. However,
any replies to the *initial* email (which is not generated by GitHub) will not
route to the GitHub issue conversation. The more quickly the **Primary** creates
the associated GitHub issue in response to the *initial* email, the less likely
this can occur. In addition, boilerplate guidance in an issue template will help
to mitigate this by informing ``visit-users`` members of this behavior.

For any work the **Primary** engages in, there should be an associated issue
for tracking that work.

.. danger::
   What about ``visit-help-asc`` email list? Should we handle similarly?
   Can we just foward to ``visit-users``?  If no to either, what do we do?

.. _sre_response_vs_resolution:

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
their direct collaborators are accustomed to response times of less than four
hours. For example, the VisIt_ project operates a telephone hotline and also
frequently handles walk-ins. As an aside, after a recent small test effort to
maintain a rapid response time, a noticeable up-tick in user email inquiries was
observed suggesting that rapide response times have the effect of encouraging
more user interactions.

It is also important to distinguish between *response* and *resolution* here.
A key goal in this process is to ensure that customer inquires do not go
unanswered for a long time. However, *responding* to a customer inquiry does
not necessarily mean *resolving* it. Sometimes, the only response possible is to
acknowledge the customer's inquiry and let them know that the resources to
address it will be allocated as soon as practical. In many cases, an *immediate*
response to acknowledge even just the receipt of a customer's inquiry with no
progress towards actual resolution goes a long way towards creating the goodwill
necessary to negotiate a day or more of time to respond more fully and maybe even
resolve.

*Resolution* of an SRE issue often involves one or more of the
following activities...

  * Answering a question or referring a user to documentation.
  * Diagnosing the issue.
  * Developing a work-around for users.
  * Developing a reproducer for developers.

    * This may include any relevant user data files as well as approval, where
      appropriate for world read access to such data.

  * Identifying a *low-hanging fruit* type issue and subsequently engaging
    in the *housekeeping* work to resolve it.
  * Determining if the user's issue is known (e.g. an issue ticket already exists).
  * Updating a known issue with new information from this user, perhaps
    adjusting labels on the issue or putting the issue back into the
    UN-reviewed state for further discussion at a VisIt_ project meeting.
  * Identifying and filing a new *product development* type issue ticket.

To emphasize the last bullet above, *resolution* of an SRE
issue does not always mean a customer's issue can be addressed to *satisfaction*
within the constraints of the processes defined here. Sometimes, the most
that can be achieved is filing a highly informative issue ticket to be
prioritized, scheduled and ultimately resolved as part of normal VisIt_ product
development activities. Nonetheless, doing so does serve to *resolve* the
original SRE issue that initiated the work.

.. _sre_handoffs:

Handoffs
--------

Our SRE processes involve two kinds of *handoffs*. One is the
redirection of a customer who makes contact with a developer not serving as the
**Primary**. The other is the handoff of unresolved SRE issues
from one week's **Primary** to the next.

To handle customer redirection handoffs, it is a best practice to use a three-way
handoff giving the customer some assurance that their initial contact with someone
is successfully handed off to the **Primary**. For example, for a call-in, it
is a best practice to try a three-way call transfer. For some developers, the
propspect of redirecting friends and colleagues with whom they may have long
standing relatinships may be initially uncomfortable. But it is important to
recognize that this an essential part of achieving one the goals of this process,
to reduce SRE interruptions for the team as a whole.

If an active SRE issue cannot be resolved within the week of
a **Primary**'s assignment, it gets handed off to the next week's **Primary**.
Such handoffs shall be managed formally with a comment (or email) to the
customer(s) and the next week's **Primary** and **Backup** in the associated
GitHub issue. The associated issue(s) in the SRE issues
repository shall be re-assigned by the next week's **Primary** upon beginning
their shift.

.. _sre_escalation:

Escalation
----------
SRE inquiries may escalate for a variety of reasons. The 
technical expertise or authority required may be beyond the **Primary**'s
abilities or other difficulties may arise. For issues that the **Primary** does
not quickly see a path to resolution, the **Backup** should be enlisted first.
When developer expertise other than **Backup** is needed, the **Primary** should
try to engage other developers using the ``@`` mention feature in the associated
GitHub issue. However, where a **Primary** is responsible for maintaining the 
response time goal, other developers so enlisted are free to either delay or even
decline to respond (but nonetheless inform the **Primary** of this need) if their
schedule does not permit timely response. Such a situation could mean that the
only remaining course of action for the **Primary** to *resolve* the issue is to
file a product development issue as discussed at the end of a preceding section.

If after investigation and diagnosis the work required to resolve an SRE
incident remains highly uncertain or is not believed to be a
*low-hanging-fruit* type task, the **Primary** should search the issue system to
see if this is a known issue and, if so, add additional information to that known
issue about this new SRE incident (and perhaps remove the
*reviewed* tag from the issue to cause the issue to be re-reviewed at the next
VisIt_ project meeting) or submit a *new* issue to the product development issue
tracker. Such action then *resolves* the original SRE issue.

.. _sre_scf_issues:

Special Considerations for Classified Computing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Occasionally, incidents arise that may be handled only in the Secure Computing
Facility (SCF). This is not too common but does happen and it presents problems
for a geographically distributed team. In many ways, an SCF-only incident is just
a different form of *escalation*.

On the one hand, customers on SCF are accustomed to longer response times.
On the other hand, often work on the SCF is a high priority and requires
rapid response from a developer that is on site with access to SCF.

Our current plan is to handle this on a case-by-case basis. If neither the
**Primary** nor **Backup** are able to handle a customer response incident
requiring the SCF, the **Primary** should

  * First determine the customer's required response time. It may be hours
    or it may be days. If it is days. Its conceivable the issue could be
    handled in the following week by a new **Primary/Backup** pair.
  * If customer indicates immediate response is required, the **Primary**
    should inquire the whole team to arrange another developer who can
    handle it.

.. _sre_housekeeping:

Housekeeping and Low-hanging Fruit Type Issues
----------------------------------------------

Part of the reason for developing this process is the acknowledgment of the
existence of a different category of work,
`Site Reliability Engineering (SRE) <https://en.wikipedia.org/wiki/Site_Reliability_Engineering>`_,
that is an essential part of maintaining the overall quality of a software
product as well as the productivity of both developers and users of the
software alike.

Issues that impact one user's productivity often impact others. Likewise for
developer productivity issues. Often, these kinds of issues can wind up falling
through the cracks of traditional software project management and planning
processes. However, such issues also often represent low cost high benefit
improvements in quality of either the software itself or the development
or deployment processes supporting it. We refer to issues of this nature
as general *housekeeping* or *low-hanging fruit* type issues.

Apart from acknowledging their existence, a key part of this process is the
allocation of resources for the sole purpose of supporting SRE activities
and developing a practice of continuously resolving
general housekeeping or low-hanging fruit type issues arising from
SRE inquiries.

Consequently, another key role of the **Primary** is to use any time not working
active inquiries to fix *low-hanging fruit* issues; either those the **Primary**
is currently managing or those from the *backlog*. As a rule of thumb, low-hanging
fruit is considered to be anything that the **Primary** believes is fixable
within a half-day's (4 hours) worth of effort. When there are many such tasks in
the system to work on, the **Primary** is free to use his/her judgment to decide
which s/he can most productively address.

Part of the acknowledgment of this new category of work is the new issue tracker
for tracking it. *New* SRE activity will start with an issue 
being added in this new issue tracker. However, there are likely a number of
issues of this same kind already mixed in with our *normal* product development
issues backlog. These should probably be audited for whether or not they are
an issue of the *general housekeeping* or *low-hanging fruit* type here and
then appropriately re-labeled.

.. danger::
   The whole team should engage in a label-palooza to sift through existing
   issues in tracker and identify those of this type. With ~1600 issues and
   eight people, we each can take about 200 issues in the tracker.

.. _sre_scheduling:

Scheduling and Load Balancing
-----------------------------

To balance the work load of SRE, the responsibilities of the
**Primary** and **Backup** are rotated, round-robin among team members. For
example, on a team of eight developers, each would serve as **Primary** only one
week in eight or 12.5% of their time. However, a number of factors complicate
this simple approach including percent-time assignments of team members,
alternate work schedules, working remotely, travel, vacations, trainings,
meetings, etc.

Round-robin assignment may lead to a fair load by head-count but isn't weighted by
percent-time assignments. From a percent-time assignment perspective, it might be
more appropriate for a developer that is only 50% time on VisIt_ to serve as the
**Primary** only half as often as a 100% time developer.

Since a majority of VisIt_ developers divide their time across multiple projects,
we use 50% as the *nominal* developer assignment. Because of all the factors that
can effect scheduling, the VisIt_ project has opted to manage scheduling by
periodically negotiating assignments 1-3 months into the future and recording the
assignments on a shared calendar. The aim is an approximately round-robin load
balancing where contributors who are more than 50% time on VisIt_ are occasionally
assigned an extra week. Either **Primary** or **Backup** can make last minute
changes to the schedule by finding a willing replacement, updating the shared
calendar and informing the rest of the team of the change.

Whenever possible, an experienced **Backup** will be scheduled with a less
experienced **Primary**.

.. _sre_misconceptions:

A Common Misconception: SRE is an Interruption to Programmatic Work
-------------------------------------------------------------------
When faced with a long backlog of development tasks, team members can all too
easily perceive SRE work as an *interruption* to those tasks.
This is a common misconception. SRE is an important aspect to
a successful product and project on par with any other major development work.
It is part of what is involved in keeping the software working and useful tool
in our customer's workflows not only here at LLNL, likely VisIt_'s biggest
customer, but wherever in DOE/DOD and elsewhere in the world VisIt_ is used.

Indeed, there are several advantages in having developers involved with
SRE activities. These include..

  * Learning what problems users are using the tool to solve.
  * Learning how users use the tool.
  * Learning what users find easy and what users find hard about the tool.
  * Learning where documentation needs improvement.
  * Learning where the user interface needs improvement.
  * Learning operational aspects of user's work that the tool can impact.
  * Building collaborative relationships with other members of the organization.
  * Learning how users operate in performing their programmatic work for the
    organization which helps to inform planning for future needs.

In short, the work involved in Software Reliability Engineering (SRE) and
ensuring productivity of both users and developers of VisIt_ *is* programmatic
work. The practice of having software development staff *integrated* with
*operations* is more commonly referred to as *DevOps*. There is a pretty good
`video <https://youtu.be/XoXeHdN2Ayc>`_ that introduces these concepts.
