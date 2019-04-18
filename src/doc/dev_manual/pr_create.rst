Creating a Pull Request
=======================

Overview
--------

Pull Requests (PRs) allow developers to review work before merging it into the
develop branch. PRs are extremely useful for preventing bugs, enforcing coding
practices, and ensuring changes are consistent with VisIt_'s overall architecture. 
Because PR reviews can take time, we have adopted policies to help tailor the
review effort and balance the load among developers. We hope these policies will
help ensure PR reviews are completed in a timely manner. The benefits of reviews
outweigh the added time.  

Working with the Template
-------------------------

PR submissions are pre-populated with a template to help guide the content.
Developers do not have to use this template. Keep in mind, however, that
reviewers need structured context in order to accurately and quickly review
a PR. S, it is best to use the template or something very similar to it. The
text sections in the template are designed to be **replaced** by information
relevant to the work involved. For example, replace a line that says
*Please include a summary of the change* with an actual summary of the change.

In general, if part of the template is not relevant, please delete it before
submitting the PR. For example, delete any items in the :ref:`checklist <checklist>`
that are not relevant.

If additional structured sections in the PR submission are needed, please
use `GitHub markdown <https://guides.github.com/features/mastering-markdown/>`_
styling.

In the sections below, we describe each of the sections of the PR template in
more detail.

Description
~~~~~~~~~~~

GitHub supports a number of
`idioms and keywords <https://help.github.com/en/articles/closing-issues-using-keywords<`_
in PR submissions to help automatically link related items. Please use them.

For example, when typing a hashtag (``#``) followed by a number or text, a search menu will
appear providing potential matches based on issue or PR numbers or headlines. Sometimes no
matches will be produced even if the number being entered is correct.

If a PR is unrelated to a ticket, please delete the "Resolves #..." line for clarity.

Type of Change
~~~~~~~~~~~~~~

Bug fixes, features, and documentation improvements are among the most common
types of PR's. You may select from the menu by replacing the space between the
square brackets (``[ ]``) with an uppercase X, so that it looks exactly like
``[X]``. You can also make this selection *after* submitting the PR by checking
the box that appears on the submitted PR page.

If "Other" is checked, please describe the type of change in the space below.

Testing
~~~~~~~

Replace the content of this section with a description of how the change was tested.


.. _checklist:

The Checklist
~~~~~~~~~~~~~

The Checklist serves as a list of suggested tasks to be performed before
submitting the PR. Those that have been completed should be chedked off.
Any items that do not relate to the PR should be deleted. For example, if
the PR is not for a bugfix or feature, adding a test may not be required
and this checklist item *should* be deleted.

Notes
~~~~~

The template ends with two notes. The first is a note to squash merge the PR when it is ready to be merged. This allows you to turn several commits for one branch into one descriptive commit that encapsulates the work done on the branch. This keeps our git history clean and short. The second is a note to wait for the CI testing to pass before merging the PR. This is crucial because *if a CI test fails, then the branch should not be merged.* CI tests take approximately 30 minutes to complete.


.. choose-a-reviewer:

Reviewers
---------

If you are not an owner, then GitHub will not allow your PR to be merged into develop without a reviewer's approval, so you will need at least one reviewer. If you are an owner, then you can merge the PR into develop without review, but that doesn't necessarily mean you should. Follow these guidelines to determine the need and number of reviewers. Note, these guidelines serve as a "lower bound"; you may always add more reviewers to your PR if you feel that is necessary.


No Reviewers (owners only)
~~~~~~~~~~~~~~~~~~~~~~~~~~

If your changes are localized and you have satsified all the testing requirements and you are confident in the correctness of your changes (where correctness is measured by both the correctness of your code for accomplishing the desired task and the correctness of *how* you implemented the code according to VisIt_'s standard practices) then you may merge the PR without a reviewer *after* the CI tests pass.


One reviewer
~~~~~~~~~~~~

If your changes have a broader impact or you are working in an area of VisIt_ with which you are not familiar or you are changing existing behavior, then you should add a reviewer.

If you are not an owner, then you must always have at least one reviewer, even if you satisfy the guidelines for the "No Reviewers" case. 


Two or more reviewers
~~~~~~~~~~~~~~~~~~~~~

If your changes substantially modify existing behavior or you are updating significant amounts of the code or you are designing new architectures or interfacess, then you should have at least two reviewers.


Choosing Reviewers
~~~~~~~~~~~~~~~~~~

GitHub automatically suggests reviewers based on the blame data for the files you have modified. So unless you have a specific need for a specific reviewer, then you're probably safe to go with GitHub's suggestions.

.. developer-process:

Iteration Process
-----------------

Review processes are iterative by nature, and PR reviews are no exception. A typical review process looks like this:

#. You submit a pull request and select a reviewer.
#. The reviewer goes through and leave comments, suggestions, and tasks for you to do.
#. You get clarification for anything that us unclear and update your PR according to the suggestions.
#. Repeat steps 2 and 3 until the reviewer is satisfied with the PR.
#. The reviewer approves the PR.

The actual amount of time it takes to perform a review or update the changes is relatively small compared to the amount of time the PR *waits* for the next step in the iteration. The wait time can be exacerbated in two ways: (1) The reviewer or developer is unaware that the PR is ready for the next step in the iteration process, and (2) the reviewer or developer is too busy with other work. To help alleviate the situation, we recommend the following guidelines for the developer (guidelines for the reviewer can be found `here <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/dev_manual/pr_review.html#iteration-process>`_).

* Make sure your code is clear and well commented and that your PR is descriptive. This helps your reviewers quickly familiarize themselves with the context of your changes. If code is unclear, the reviewers may spend a lot of time trying to grasp the purpose and effects of your changes.
* Immediately answer any questions your reviewers ask about the PR. You should turn on notifications for this.
* When the reviewers have finished reviewing (step 2), make it a top priority to update your code according to the requested changes. Use the @<username> feature to notifiy the reviewers of any questions you have about the suggestions/comments.
* When you have finished updating your PR (step 3), leave a comment on the PR using @<username> to let the reviewers know that the PR is ready to be looked at again.
* Reviewers won't always merge a PR after they approve it, especially if the squash-merge involves combining many commits into a single, descriptive comment. When your PR is approved, you should squash-merge to develop with a succinct description.
