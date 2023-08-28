.. _Creating a Pull Request:

Creating a Pull Request
=======================

Overview
--------

Pull Requests (:abbr:`PR (Pull Request)`\s) allow developers to review work before
merging it into the develop branch. PRs are extremely useful for preventing bugs,
enforcing coding practices, and ensuring changes are consistent with VisIt_'s overall
architecture. Because PR reviews can take time, we have adopted policies to help
tailor the review effort and balance the load among developers. We hope these policies
will help ensure PR reviews are completed in a timely manner. The benefits of reviews
outweigh the added time.

Forking the repo
----------------

Developers who do not have write access to the primary VisIt_ repo may make
contributions by forking the repo and submitting pull requests. GitHub provides
excellent informational articles about `forking a repo <https://help.github.com/en/articles/fork-a-repo>`_ and
`creating pull requests from a fork <https://help.github.com/en/articles/creating-a-pull-request-from-a-fork>`_.

Working with the Template
-------------------------

:abbr:`PR (Pull Request)` submissions are populated with a template to help guide
the content. Developers do not have to use this template. Keep in mind, however, that
reviewers need structured context in order to accurately and quickly review
a PR. So, it is best to use the template or something very similar to it. The
text sections in the template are designed to be *replaced* by information
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
`idioms and keywords <https://help.github.com/en/articles/closing-issues-using-keywords>`_
in :abbr:`PR (Pull Request)` submissions to help automatically link related items.
Please use them.

For example, when typing a hashtag (``#``) followed by a number or text, a search
menu will appear providing potential matches based on issue or PR numbers or
headlines. Sometimes no matches will be produced even if the number being entered
is correct, but the link will still occur when the PR is submitted. By placing the
keyword "Resolves" in front of a link to an issue, the issue will automatically
close when the PR is merged.

If a PR is unrelated to a ticket, please delete the "Resolves #..." line for clarity.

Type of Change
~~~~~~~~~~~~~~

Bug fixes, features, and documentation improvements are among the most common
types of :abbr:`PR (Pull Request)`\s. You may select from the menu by replacing
the space between the square brackets (``[ ]``) with an uppercase X, so that it
looks exactly like ``[X]``. You can also make this selection *after* submitting
the PR by checking the box that appears on the submitted PR page.

If "Other" is checked, please describe the type of change in the space below.

Testing
~~~~~~~

Replace the content of this section with a description of how the change was tested.


.. _checklist:

The Checklist
~~~~~~~~~~~~~

The Checklist serves as a list of suggested tasks to be performed before
submitting the :abbr:`PR (Pull Request)`\. Those that have been completed should
be checked off. Any items that do not relate to the PR should be deleted. For
example, if the PR is not for a bugfix or feature, adding a test may not be
required and this checklist item *should* be deleted.


.. choose-a-reviewer:

Reviewers
---------

GitHub will not allow non-owners to merge :abbr:`PR (Pull Request)`\s into develop
without a reviewer's approval. Non-owners will need at least one reviewer. Owners
may merge a PR into develop without review. But, that does not necessarily mean
they should. Follow the guidelines below to determine the need for and number of
reviewers. Note, these guidelines serve as a "lower bound"; you may always add more
reviewers to your PR if you feel that is necessary.


No Reviewers (owners only)
~~~~~~~~~~~~~~~~~~~~~~~~~~

If your changes are localized, you have satisfied all the testing
requirements and you are confident in the correctness of your changes
(where correctness is measured by both the correctness of your code for
accomplishing the desired task and the correctness of *how* you implemented
the code according to VisIt_'s standard practices) then you may merge the
:abbr:`PR (Pull Request)` without a reviewer *after* the CI tests pass.


One reviewer
~~~~~~~~~~~~

If the changes have a broader impact or involve an unfamiliar area of VisIt_
or existing behavior is being changed, then a reviewer should be added.

Non-owners must always have at least one reviewer even if you satisfy all other
guidelines for the *No Reviewers* case.


Two or more reviewers
~~~~~~~~~~~~~~~~~~~~~

If your changes substantially modify existing behavior or you are updating
significant amounts of the code or you are designing new architectures or
interfaces, then you should have at least two reviewers.


Choosing Reviewers
~~~~~~~~~~~~~~~~~~

GitHub automatically suggests reviewers based on the blame data for the files
you have modified. You should choose the GitHub suggested reviewer unless you
have a specific need for a specific reviewer.

Iteration Process
-----------------

Review processes are iterative by nature, and :abbr:`PR (Pull Request)` reviews
are no exception. A typical review process looks like this:

#. The developer submits a PR and selects a reviewer.
#. The reviewer reviews the PR and writes comments, suggestions, and tasks.
#. The developer gets clarification for anything that us unclear and updates the PR according to the suggestions.
#. Repeat steps 2 and 3 until the reviewer is satisfied with the PR.
#. The reviewer approves the PR.

The actual amount of time it takes to perform a review or update the PR
is relatively small compared to the amount of time the PR *waits* for the next
step in the iteration. The wait time can be exacerbated in two ways: (1) The
reviewer or developer is unaware that the PR is ready for the next step in the
iteration process, and (2) the reviewer or developer is too busy with other work.
To help alleviate the situation, we recommend the following guidelines for the
developer (guidelines for the reviewer can be found
`here <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/dev_manual/pr_review.html#iteration-process>`_).

* Make sure the code is clear and well commented and that the PR is descriptive. This helps the reviewers quickly familiarize themselves with the context of the changes. If the code is unclear, the reviewers may spend a lot of time trying to grasp the purpose and effects of the PR.
* Immediately answer any questions the reviewers ask about the PR. Enabling notifications will help speed this along.
* When the reviewers have finished reviewing (step 2), quickly update the PR according to the requested changes. Use the ``@username`` idiom to notify the reviewers for any clarification
* When you have finished updating your PR (step 3), write a comment on the PR using ``@username`` to let the reviewers know that the PR is ready to be looked at again.
