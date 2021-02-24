Reviewing a Pull Request
========================

Overview
--------

Pull Requests (:abbr:`PR (Pull Request)`\s) allow developers to review work before
merging it into the develop branch. PRs are extremely useful for preventing bugs,
enforcing coding practices, and ensuring changes are consistent with VisIt_'s overall
architecture. Because PR reviews can take time, we have adopted policies to help
tailor the review effort and balance the load among developers. We hope these policies
will help ensure PR reviews are completed in a timely manner. The benefits of reviews
outweigh the added time.


Checklist
---------

In the course of reviewing a :abbr:`PR (Pull Request)`, the reviewer should use the
following as a checklist. The reviewer should verify that any deleted items are
rightfully so.

* The developer followed Visit_'s style guidelines
* The developer commented the code, particularly in hard-to-understand areas
* The developer updated the release notes
* The developer made corresponding changes to the documentation
* The developer added debugging support
* The developer added tests that prove the fix is effective or that the feature works
* The developer has confirmed new and existing unit tests pass
* The developer has NOT changed any *protocol* or public *interfaces* on an RC branch
* If necessary, the developer added any new baselines to the repository

These reminders will appear as checklist items in the :abbr:`PR (Pull Request)` template.
However, not all items *apply* in all PRs. For the items that do apply be sure you have
done the associated work and then check off the items by replacing the space in ``[ ]``
with an ``x`` (or if you prefer you can submit the PR and then check the boxes with the
mouse). For items that do not apply, be sure to change these lines to strikeout style by
adding ``~~`` just before the check box ``[ ]`` (but after the bullet ``-``) and also at
the end of the line like so::

    - [ ] This item is unchecked.
    - [x] This item is checked.
    - ~~[ ] This item has been striken out.~~

Comments and Tasks
------------------

GitHub provides two ways to add comments to the :abbr:`PR (Pull Request)`.


Generic Comments
~~~~~~~~~~~~~~~~

The first type of comment is a generic :abbr:`PR (Pull Request)` comment for
communicating about general things related to the changes or the PR process. This
comment box is found at the bottom of the "Conversation" tab, which is the main
tab on the PR page. The reviewer should use this when pinging the developer to
update changes (see :ref:`Iteration Process <iteration-process>` below).


Code Related Comments
~~~~~~~~~~~~~~~~~~~~~

The "Files changed" tab in the :abbr:`PR (Pull Request)` will show a diff of all the
changes. Hover the mouse over the white space to the right of the line number and a
blue plus sign will appear. Click this and a comment box will pop up. Type any comments
and click either "Add single comment" or "Start a review" (see
:ref:`Review Changes <review-changes>` for more information). This type of comment
can be used to ask specific questions or suggest specific changes to the PR.


.. _review-changes:

Review Changes
--------------

In addition to comments, the reviewer should also explicitly mark the state of the
:abbr:`PR (Pull Request)`. There are two ways to do this.

Upon writing a code related comment, select the "Start a review" button. This will
initiate a review. Click "Add review comment" for each new comment. When you are
done, navigate to the top-right of the page and click "Finish your review".

Alternately, the reviewer can first write all the comments and then submit a review.
Use the "Add single comment" button for each code related comment. Then, once you
have finished commenting, navigate to the top-right of the page and click "Finish
your review".

Upon clicking the green "Finish your review", GitHub will present the ability to
add additional generic comments and to update the state of the PR. If you left
comments via the "Add single comment" button, then you *must* add an additional
comment here to be able to submit a review. These are the three options for
updating the PR:

#. Comment - Submit general feedback without explicit approval. This is ambiguous and should not be used because the developer does not always know if the reviewer think changes should be made. It does not update the state of the PR.
#. Approve - Submit feedback and approve merging these changes. Use this when the PR is ready to be merged into develop.
#. Request changes - Submit feedback that must be addressed before merging. Use this when the developer should make additional changes to the PR.


.. _iteration-process:

Iteration Process
-----------------

Review processes are iterative by nature, and :abbr:`PR (Pull Request)` reviews
are no exception. A typical review process looks like this:

#. The developer submits a pull request and selects a reviewer.
#. The reviewer writes comments and submit a "Request change" review or an "Approve" review.
#. The developer updates the PR according to the suggestions.
#. Repeat steps 2 and 3 until the PR is ready.
#. The reviewer approves the PR.

The actual amount of time it takes to perform a review or update the PR
is relatively small compared to the amount of time the PR *waits* for the next
step in the iteration. The wait time can be exacerbated in two ways: (1) The
reviewer or developer is unaware that the PR is ready for the next step in the
iteration process, and (2) the reviewer or developer is too busy with other work.
To help alleviate the situation, we recommend the following guidelines for the
reviewer (guidelines for the developer can be found
`here <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/dev_manual/pr_create.html#iteration-process>`_)

* Immediately address the PR. Enabling notifications will help speed this along.
* If anything in the PR is unclear, ask specific questions using generic or code related comments. Make use of the ``@username`` idiom to directly ping the developer.
* Clearly mark the review as "Approved" or "Request changes".
* Notify the developer with the ``@username`` idiom that the PR is ready for updates.
* When the developer has updated the PR, make it a top priority to review it again.
* When the PR is ready to be merged into develop, approve the PR and squash-merge the PR into develop with a succinct description of the changes.

If you are chosen as a reviewer and you know that you will not be able to review
the PR in a timely manner, please let the developer know and provide suggestions
for who to choose instead. Once you start a PR review, you should make it a
priority and stick with it until the end.
