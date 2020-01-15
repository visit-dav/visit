Live Incident Response
======================

VisIt_ software developers also have to serve in incident response roles taking
hotline calls, replying to emails, responding to drop-ins, etc. Incident
response is about the *operations* side of VisIt_ software. That is the side
associated with users *running* the software to get their work done. Apart from
the VisIt_ software itself, *operations* involves many other things including
the computing hardware, file systems and networks upon which VisIt_ software
runs as well as the security enviornment and any third party software
dependencies involved.

Most of those aspects of *operations* are typically out of the purview of the
VisIt_ team. In most cases the extent of the VisIt_ team's involvement in
*operations* is confined primarly to the VisIt_ software itself; its
development, testing, release and deployment which includes installations we
directly support on systems where we have accounts, hosted binary downloads for
common platforms and the tools to build from sources.

The practice of having software development staff *integrated* with *operations*
is more commonly referred to as *DevOps*. There is a pretty good
`video <https://youtu.be/XoXeHdN2Ayc>`_ that introduces these concepts.

This document explains our procedures for how VisIt_ software developers handle
active incidence response calls. Some of the goals of this proccess are...

  * To try to *load balance* the work of incident response in an equitable way
    across the development team.
  * To achieve a 1-hour response time.
  * To reduce incident response interruptions for the team as a whole.
  * Identify and document escalation paths for major incidents.

For the VisIt_ project, normal business hours are 8am-12pm and 1-5pm, Monday
through Friday excluding LLNL holidays. The work of *live* incidence response is
allocated in one-week intervals. During a week, one developer is assigned
as the *primary* incidence response contact and a second developer is assigned
as a *backup*. Live incidence response issues will be logged and tracked in a
separate GitHub respository within the
`visit-dav GitHub organization. <https://github.com/visit-dav>`_

The Primary's Role
------------------

The primary's role is to answer all incidents that occur during that week
including those that came in during the preceding weekend/holiday. Other
developers who may be contacted directly by customers should redirect such
inquires to the primary. The primary's goal is to *wrap-up* all incidents
by the end of their week. If an issue cannot be wrapped-up, it gets handed-off
to next week's primary. Handoff's of incidents are managed formally with
an email to customer and the next week's primary and backup contact information.

For each new incident, the primary should file an issue ticket in the
`visit-live-incidents <https://github.com/visit-dav/visit-live-incidents/issues>`_
repo and assign themselves. When the incident is resolved, the associated issue
should be closed. Primary should endeavour to capture all relevant information
and communications involving the incident in this issue. For users who do not have
GitHub accounts, this will require the primary to do a modest amount of cutting
and pasting between email and GitHub issues.

The goal of the primary is to reach a *resolution* of each incident. Resolution
often involves one or more of the following activities.

  * Answering a question or refering user to documentation.
  * Determining if user's issue is known (e.g. an issue ticket already exists).
  * Updating a known issue with additional information from this user, perhaps
    reprioritizing the issue or putting the ticket back into the UN-reviewed
    state for further discussion at a VisIt_ project meeting.
  * Identifying and subsequently fixing a *low-hanging fruit* issue.
  * Developing a reproducer for developers.
  * Developing a work-around for users.
  * Identifying and filing a coding *new* issue in the main source repo.

Another role of the primary is to use their time during the week to fix any
*low-hanging fruit* issues revealed by an incident. For example, if an incident
reveals incorrect or missing documentation, the primary should work to fix such
issues. As a rule of thumb, low-hanging fruit is considered to be anything that
primary believes is fixable within a half-day's (4 hours) worth of effort. 

When primary has challenges with incident diagnosis, s/he should feel free to
engage other developers with help starting with the *backup*. When developer
expertise other than *backup* is needed, primary should arrange mutually
agreeable times to engage those developers.

There shall be no expectation that a developer serving as *primary* can get any
other work done beyond their incidence response obligations. In slow weeks, its
concievable they can. But, there can be no implied assumption or expecation
that this will be the case. Furthermore, even in a slow week, the *primary* may
likely be able to find plenty of *low-hanging-fruit-type* tasks in the main
repository issues to tackle and fix.

During their communication's with the customer, the *primary* should avoid
reply-all-type communications.

For applicable incidents, the *primary* should prepare a set of bullets to
form an outline of a *post-mortem* for the incident.

The Backup's Role
-----------------

Ideally, like the rest of the development team, the backup is never called into
action for incident support. However, there are cases where *backup* may be
needed. For this reason, the *backup* is asked to at least track and maintain
knowledge and awareness of the issues the primary is supporting. A good way to
achieve this is to selectivley *watch* the visit-live-indicents repository.
In cases where *primary* needs help in resolving an incident, the *backup*
shall serve as the first line of defense. In addition, the *backup* should be
ready to step up to the *primary* role in the (hopefully rare) occurance that
*primary* is unable to maintain the one-hour response time goal. To the extent
possible, the transition from *backup* to *primary* should be managed formally.
Otherwise, the *backup* should not engage in incident response activities.

In the short term, it will help to pair veteran *backups* with *primary*
developers who have less experience with live incident support.

Special Considerations for SCF
------------------------------

Customers on SCF are accustomed to longer response times. At the same time,
often work on the SCF is a high priority and cannot be easily delayed.
Because a portion of VisIt_ development team works from off-site, we need to
develop strategies for handling SCF-specific work. Fortunately, SCF-specific
work is not all that common.
