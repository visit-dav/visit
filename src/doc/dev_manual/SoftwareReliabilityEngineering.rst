.. _software_reliability_engineering:

Software Reliability Engineering (SRE)
======================================

In the VisIt_ project, members of the development team are frequently called
upon to respond to a variety of inquiries often originating directly from users.
Some of these may relate to the *use* of VisIt_ such as

  * How do I do a surface plot?
  * How do I compute the mass in a given material?
  * How do I get client/server to TACC working?

and some may relate to an *operational* aspect of either the VisIt_ software
itself such as

    * A botched *managed* VisIt_ installation.
    * An update to host profiles to address site access changes.
    * A missing database reader plugin.

or, the underlying computing infrastructure upon which VisIt_ depends such as

    * An incompatible graphics driver.
    * A downed file system or network.
    * A trip in the security environment.

Typically, such inquiries originate from users in the midst of using VisIt_
and are encountering some kind of difficulty. In highly effective software
projects, the work involved in handling such inquiries does not end
with fixing this one user's problem and sending them on their way. When one
user encounters a problem, there are probably others who have encountered
the same problem. Furthermore, often the problems users encounter are
suggestive of minor, easily fixed deficiencies in either the software itself
or its associated processes and artifacts.

The continuous investment of effort to craft and carry out
:ref:`small corrective actions <sre_sre_vs_product_development>`
in response to such inquiries is a *best practice*. It represents a
`fusion <https://medium.com/@aHev/why-ux-researchers-should-learn-sre-practices-a2b213e69a8a>`_
of aspects of Google's
`Site Reliability Engineering <https://landing.google.com/sre/sre-book/toc/>`__ (SRE)
process (sometimes also called
`Systems Reliability Engineering or Services Reliability Engineering <https://www.cio.com/article/3192531/why-you-need-a-systems-reliability-engineer.html>`_)
and aspects of
`User Experience Regression Testing <https://www.uxmatters.com/mt/archives/2019/04/reining-in-ux-regression.php>`_
and/or
`User Experience Driven Development (UXDD) <https://docs.microsoft.com/en-us/archive/msdn-magazine/2016/february/cutting-edge-architecture-spinoffs-of-uxdd>`_.

For mature DOE software projects with wide reach and many users, SRE activity
represents a brand of effort wholly different from conventional software product
development, planning and execution. Like most DOE software projects, VisIt_ has
no dedicated SRE resources. Instead, developers themselves must also support SRE
work. Nonetheless,
`managing SRE work effectively <https://beyondphilosophy.com/15-statistics-that-should-change-the-business-world-but-havent>`_
and efficiently is an essential part of maintaining the overall productivity and
sustainability of the software as well as the productivity of both users and developers
of the software alike.

Goals
-----

This document describes how the VisIt_ project manages its SRE activities.
Some of the goals of this process are...

  * To maintain a reputation for timely and quality response to customer inquiries.
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

Business Hours
~~~~~~~~~~~~~~

In the IT world where companies like Google, Apple and Amazon have whole teams
dedicated to SRE activity, coverage is 24/7 and response time is measured in
*minutes*. For the VisIt_ project where the majority of funded development takes
place at Lawrence Livermore National Lab, coverage is during normal West Coast
*business* hours, 8am-12pm and 1-5pm (GMT-8, San Francisco time zone), Monday
through Friday excluding
`LLNL holidays <https://supplychain.llnl.gov/poattach/pdf/llnl_holidays.pdf>`_
and response time may be as much as four hours due to team members having to
multi-task among many responsibilities.

Developer Away Notifications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As an aside but nonetheless related to the SRE response time goal of four hours,
developers agree to notify the team if, during normal business hours, they will
unexpectedly be away from their desk for periods longer than four hours. This is
true whether working on or off site. Being away for periods shorter than four
hours does not require such notification.

The Basic Process
-----------------

SRE work is allocated and rotated among developers in
one-week *shifts*. During a shift, one developer's :ref:`role <sre_roles>` is to
serve as the **Primary** SRE contact and a second developer's
:ref:`role <sre_roles>` is to serve as a **Backup**. Except for
:ref:`escalations <sre_escalation>`, all other developers are free of SRE
responsibilities for that week.

The :ref:`role <sre_roles>` of the **Primary** is to :ref:`respond <sre_response_vs_resolution>`
within the response time goal, to each inquiry. Ideally, all SRE
activity during the week is handled and :ref:`resolved <sre_response_vs_resolution>`
solely by the **Primary**. However, :ref:`escalations <sre_escalation>`, which we
hope are rare, will wind up engaging the **Backup** and may even engage other
developers. In addition, any :ref:`active SRE dicussions <sre_active_discussions>` that remain
unresolved at the end of the week are formally :ref:`handed off <sre_handoffs>` to
the next **Primary**.

:ref:`Active SRE discussions <sre_active_discussions>` will be logged and tracked in a separate GitHub,
`issues-only repository <https://github.com/visit-dav/live-customer-response/issues>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. Upon
resolution of :ref:`serious incidents <sre_serious>`, the **Primary** will prepare a brief
*postmortem* to inform a discussion at the next project meeting of possible changes
in practices to avoid repeating such major incidents.

Because SRE work tends to be :ref:`interrupt driven <sre_misconceptions>`, there is always
the chance that the **Primary** will have no :ref:`active <sre_active_discussions>` discussions.
At these *idle* times, the **Primary** shall use their time to address general
:ref:`housekeeping <sre_sre_vs_product_development>` or other *low-hanging fruit* type work. In
particular, there shall be no expectation that a developer serving as **Primary**
can get any other work done beyond their active or idle SRE obligations. In slow
weeks, its conceivable they can. But, there can be no implied assumption or
expectation that this will be the case.

A :ref:`schedule <sre_scheduling>` of the **Primary** and **Backup** assignments going
out several months is periodically negotiated by the team and posted in the form
of a shared calendar. **Primary** and **Backup** responsibilities are rotated
so as to balance the load among team members.

The preceding paragraphs describe VisIt_'s SRE processes at a
basic level and in the ideal. Nonetheless, several terms here (those that are 
links or in *italics* in the paragraphs above) require elaboration. In addition,
there are also many practical matters which can serve to complicate the basic
process. These details are addressed in the remaining sections.

.. _sre_roles:

Roles
-----

The **Primary**'s role is to respond, within the response time goal, to each
inquiry that occurs during that week including those that come in during the
preceding weekend/holiday. The **Primary**'s goal is to :ref:`resolve <sre_response_vs_resolution>`
all inquiries by the end of their week.

The **Primary** has the sole responsibility for responding to inquiries
and opening and resolving :ref:`SRE issue tickets <sre_active_discussions>`.
When the **Primary** needs help to
:ref:`resolve an SRE issue <sre_response_vs_resolution>`, s/he should
first enlist the **Backup**. This is an :ref:`escalation <sre_escalation>`.
Nonetheless, the **Backup** (or other developers for that matter) are called
into action only by explicit request of the **Primary**.
Note that enlisting additional resources for help is part of
:ref:`escalation <sre_escalation>` and is not the same as a
:ref:`handoff <sre_handoffs>`.

If the **Primary**'s schedule changes such that the response time goal may
not be met, the **Primary** may temporarily *delegate* his/her role and
responsibilities to the **Backup**. To the extent possible, such temporary
delegation from **Primary** to **Backup** should be handled formally and by
mutual agreement. Temporary delegation of the **Primary**'s role is also
not the same as a :ref:`handoff <sre_handoffs>`.

Ideally, the **Primary** is able to handle all SRE activity
and no other developers are engaged. Thus, other developers are free to
ignore customer inquiries as well as redirect customers who may contact them directly
via email, phone or walk-in. It is a best practice to handle such redirections
with a formal, three-way :ref:`handoff <sre_handoffs>` confirming that the customer
indeed makes contact with the **Primary**.

.. _sre_sre_vs_product_development:

SRE vs. Product Development
---------------------------

Part of the reason for formalizing this process is the recognition of a
different category of work,
`Site Reliability Engineering <https://en.wikipedia.org/wiki/Site_Reliability_Engineering>`__ (SRE),
that is
`essential part of maintaining the overall quality <https://beyondphilosophy.com/15-statistics-that-should-change-the-business-world-but-havent>`_
of a software product as well as the productivity of both developers and users
of the software alike. Nonetheless, SRE work is very different from
conventional *product development* type work where bug fixes, technology
refreshes and feature enhancements are estimated and prioritized,
methodically planned and resources are assigned to hit target release dates.

Issues that impact one user's productivity often impact others. Likewise for
developers. When such issues come to our attention, whenever possible it
is often helpful to identify *two* kinds of actions; a short-term
*constructive* correction and a longer-term *comprehensive* solution.

=======================  ======================
Constructive Correction  Comprehensive Solution
=======================  ======================
Short term               Longer term
Faster response          Slower response
Low cost/benefit         Higher cost/benefit
Low risk                 Higher risk
Unplanned                Planned
Mitigation               Resolution
=======================  ======================

A constructive correction has value only when it represents a step towards the
comprehensive solution, can sufficiently reduce the impact of the issue and can
be rolled out to users significantly sooner and with lower cost than the
comprehensive solution. Ordinarily, a constructive correction is something the
**Primary** handles as part of their SRE activity. The comprehensive solution,
which often involves more planning and resource allocation, is handled as part
of normal product development activities.

Constructive corrections can wind up falling through the cracks of traditional
software project management and planning processes. However, such work also often
represents low cost high benefit improvements in quality of either the software
itself or the development or deployment processes supporting it. We refer to issues
of this nature as general *low-hanging fruit* type issues.

Apart from acknowledging their existence, a key part of this process is the
allocation of a small fraction of our resources for the sole purpose of supporting
SRE activities and developing a practice of continuously crafting constructive
corrective actions arising from SRE inquiries.

Consequently, another key role of the **Primary** is to use any time not working
active SRE issues to fix other *low-hanging fruit* issues from the
*product development* backlog. As a rule of thumb, low-hanging
fruit is considered to be anything that the team believes is fixable
within a half-day's (4 hours) worth of effort. When there are many such tasks in
the system to work on, the **Primary** is free to use his/her judgment to decide
which s/he can most productively address.

Part of the acknowledgment of this new category of work is the new
`issue tracker <https://github.com/visit-dav/live-customer-response/issues>`_
for tracking it. *New* SRE activity will start with an issue being added there.
As an SRE incident unfolds it may result in either the same issue being moved to the
`product development <https://github.com/visit-dav/visit/issues>`_ issue tracker
and/or new issue(s) being added to the 
`product development <https://github.com/visit-dav/visit/issues>`_ tracker.
Any new *product development* issues should be linked back to the original
SRE issue that spawned them.

.. _sre_active_discussions:

Active SRE Discussions
----------------------

:ref:`Active SRE issues <sre_active_discussions>` will be logged and tracked as discussions
in our `GitHub Discussions <https://github.com/visit-dav/visit/discussions>`_
within the `visit-dav GitHub organization <https://github.com/visit-dav>`_. 
For each new inquiry, a discussion will be created either by the primary or by the
customer who started the discussion.

The primary will endeavor to capture all relevant information and
communications in this issue. The use of GitHub Discussions for this purpose has a number
of advantages over other options such as email including better search/browse as well
as support for attachments.
For the remainder of this document we simply use the term *conversation* to refer to the
communication involved in an active SRE issue.

Upon receiving a *new* inquiry, if the inquiry did not start as a discussion (e.g. maybe
it was a telephone hotline call, or a walk-in, the procedure is for the **Primary** to include
the initial information (with the exception of inquries involving classified information) in a
new GitHub `discussion <https://github.com/visit-dav/visit/discussions>`_,
attach the SRE label and from then on handle all communication through the *conversation* associated with
that discussion.

For any work the **Primary** performs, even if it is a rather trivial amount
of work to resolve, there should be an associated discussion for tracking that work.
Tracking even the trivial tasks will help to build a database of activity we
may be able to later mine to identify patterns and further process improvements.

An SRE discussion is *answered* when the associated inquiry is
:ref:`resolved <sre_response_vs_resolution>`. Or, it is *answered* and labeled
``wontfix`` if 21 days pass since the user last engaged in any conversation with
VisIt_ developers to reach a resolution. Because of the manner in which the
interface to GitHub discussions behaves, the natural chain of communication may
not always lend itself well to using GitHub's *check this reply as the answer*
feature to indicate an SRE discussion is resolved. In such circumstances, the
adopted approach is to add a *new*, top-level comment with a link to whichever
previous, embedded comment best answers the issue and then tag that new, top-level
comment as the discussion's answer.

.. _supported_methods_of_contact:

Supported Methods of Contact
----------------------------

An SRE inquiry with the VisIt team begins with a *first contact* and may optionally
be followed by *ongoing* conversation. These two kinds of communication have different
requirements and can involve different processes. This is due to the fact that we need
to balance two priorities; *accessibility* for users and *productivity* for developers.

To maximize accessibility for users, we should support a wide variety of methods of
first contact. However, to maximize productivity for developers, we should restrict
methods of ongoing conversations.

A key benefit of having the VisIt_ team *co-located* with our user community is
that users can spontaneously make a first contact with any one of us by an office
drop-in or a tackle in the hallway or parking lot. This can even occur on social
media platforms such as Confluence, Jabber, MS Teams, etc. where users can wind
up engaging specific VisIt_ developers that happen, by nothing more than
coincidence, to also be using those platforms.

A challenge with these spontaneous methods of first contact is that they
inadvertently single out a specific developer who is then expected to at least
*respond* and possibly even to also *resolve* the issue. But, these actions
and the effort they involve are the responsibility of the primary SRE.
Consequently, spontaneous methods of first contact can wind up jeopardizing the
goals of our SRE process by making it difficult to track, allocate and manage
SRE effort.

Therefore, the methods of first contact we officially support are those which
engage the *whole team* instead of singling out a specific member. This
includes...

  * Creation of a `GitHub discussion <https://github.com/visit-dav/visit/discussions>`_.
  * Creation of a `GitHub issue <https://github.com/visit-dav/live-customer-response/issues>`_.
  * Telephone call to the `VisIt hotline <tel:42847>`_.

Whenever users attempt a first contact through something other than the supported
methods listed immediately above, the receiving developer should make an effort
to :ref:`handoff <sre_handoffs>` the inquiry to the primary SRE as quickly and
politely as practical.

What does it mean for a method of first contact to be *supported*? It means
there is an assurance that the particular platform is being monitored by VisIt_
team members during normal business hours such that the response time goal can
be maintained. In addition, supported methods are encouraged and promoted in
any documentation where VisIt_ support processes are discussed.

Balancing the priorites of user accessibility with developer productivity
involves a compromise on the number of platforms we make an assurance to monitor. 
Currently, this is limited to those listed above. However, the selected methods
should be periodically reevaluated. If there is some platform which seems to
be gaining popularity among users, it could either be added to the list of
supported platforms or perhaps it could be integrated with email in the same way
GitHub issue conversations have been.

.. _sre_response_vs_resolution:

Response Time and Response vs. Resolution
-----------------------------------------
The response time goal of four hours was chosen to reflect the worst case
practicalities of team members' schedules and responsibilities. For example, if
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
observed suggesting that rapid response times have the effect of encouraging
more user interactions.

It is also important to distinguish between *response* and *resolution* here.
A key goal in this process is to ensure that customer inquiries do not go
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
      appropriate for world read access to such data as part of attaching to
      a GitHub issue.

  * Identifying any *low-hanging fruit* type work that would address, even if
    only in part, the original SRE inquiry and then engaging in the
    *housekeeping* work to resolve it.
  * Determining if the user's issue is known (e.g. an issue ticket already exists).
  * Updating a known issue with new information from this user, perhaps
    adjusting labels on the issue or putting the issue back into the
    un-reviewed state for further discussion at a VisIt_ project meeting.
  * Identifying and filing a new *product development* type issue ticket.

To emphasize the last bullet, *resolution* does not always mean a customer's
issue can be addressed to *satisfaction* within the constraints of the SRE
process as it is defined here. Sometimes, the most that can be achieved is
filing a highly informative issue ticket to be prioritized, scheduled and
ultimately resolved as part of normal VisIt_ product development activities.
The SRE issue gets *promoted* to a product development issue. It is closed
in the SRE issue tracker and new issue is opened in the product development
issue tracker including a reference to the original SRE issue. Doing so does
serve to *resolve* the original SRE issue that initiated the work.

.. _sre_serious:

Serious Incidents and Postmortems
---------------------------------

Serious incidents are those that have significant productivity consequences for
multiple users and/or require an inordinate amount of resources (either time or
people or both) to diagnose, work-around and/or ultimately properly correct.

When such incidents occur, it is a best practice to spend some time considering
adjustments in processes that can help to avoid repeating similar issues in
the future.

When such incidents reach SRE resolution, the **Primary** will prepare a
brief *postmortem* (often just a set of bullet points) explaining what happened and why,
estimating the amount of resources that were needed to resolve the incident, describing key
milestones in the work to resolve the incident and suggesting recommendations for
changes in processes to prevent such incidents from being repeated. This *postmortem*
will be used to guide team discussion during a subsequent weekly project meeting.

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
prospect of redirecting friends and colleagues with whom they may have long
standing relationships may be initially uncomfortable. But it is important to
recognize that this an essential part of achieving one the goals of this process,
to reduce SRE interruptions for the team as a whole.

If an active SRE issue cannot be resolved within the week of
a **Primary**'s assignment, it gets handed off to the next week's **Primary**.
Such handoffs shall be managed formally with a comment (or email) to the
customer(s) and the next week's **Primary** and **Backup** in the associated
GitHub issue. The associated issue(s) in the SRE issues
repository shall be re-assigned by the previous week's **Primary** upon ending
their shift. However, a preceding week's **Primary** may be near enough
to :ref:`resolving <sre_response_vs_resolution>` an SRE issue that it makes
more sense for him/her to carry it completion in the following week. In this
case, s/he will leave such issues assigned to themselves.

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
*low-hanging-fruit* type task, the **Primary** should search the *product
development* issues to see if this is a known issue and, if so, add additional
information to that known issue about this new SRE incident (and perhaps remove the
*reviewed* tag from the issue to cause the issue to be re-reviewed at the next
VisIt_ project meeting) or submit a *new* issue to the product development issue
tracker. Such action then *resolves* the original SRE issue.

.. _sre_scf_issues:

Special Considerations for Classified Computing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Occasionally, incidents arise that may be specific to a classified computing
environment. This is not too common but does happen and it presents problems
for a geographically distributed team. In many ways, handling such an incident
is just a different form of *escalation*.

On the one hand, customers working in a classified computing environment
are accustomed to longer response times. On the other hand, such work is often
a high priority and requires rapid response from a developer that is on site
with classified computing access.

Our current plan is to handle this on a case-by-case basis. If neither the
**Primary** nor **Backup** are able to handle a customer response incident
requiring classified computing, the **Primary** should

  * First determine the customer's required response time. It may be hours
    or it may be days. If it is days. Its conceivable the issue could be
    handled in the following week by a new **Primary/Backup** pair.
  * If customer indicates immediate response is required, the **Primary**
    should query the whole team to arrange another developer who can
    handle it.

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
It is part of what is involved in keeping the software working and a useful tool
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
