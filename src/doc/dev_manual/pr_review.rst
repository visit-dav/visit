Reviewing a Pull Request
========================

Overview
--------

The Pull Request (PR) is a mechanism that allows other developers on the team to review your work before merging your branch into develop. It is extremely useful for preventing bugs, enforcing coding practicing, and ensuring changes are consistent with VisIt's overall vision. Although a review does slow down the developmenet process, there are several policies we put into place to ensure a review is completed in a timely manner, and the benefits of a review outweigh the added time.


Checklist
---------

In the course of reviewing a PR, you should use the following as a checklist for things to look for. This is the same checklist as the one in the template, but the developer may delete items from the list, so we've provided it here as well. You should verify that any deleted items are rightfully so.

* My code follows the style guidelines of this project
* I have performed a self-review of my own code
* I have commented my code, particularly in hard-to-understand areas
* I have updated the release notes
* I have made corresponding changes to the documentation
* I have added debugging support to my changes
* I have added tests that prove my fix is effective or that my feature works
* New and existing unit tests pass locally with my changes
* I have added any new baselines to the repo


Comments and Tasks
------------------

GitHub provides two ways to add comments to the PR.


Generic Comments
~~~~~~~~~~~~~~~~

The first type of comment is a generic PR comment where you can communicate with the developer about general things related to the changes or the PR process. This comment box is found at the bottom of the "Conversation" tab, which is the main tab for the PR. You should use this when pinging the developer to update changes (see Iteration Process below).


Code Related Comments
~~~~~~~~~~~~~~~~~~~~~

The "Files changed" tab in the PR will show you a diff of all the changes. If you hover your mouse over the white space to the right of the line number, a blue plus sign will appear. Click this and a comment box will pop up. Type your comment and hit either "Add single comment" or "Start a review" (see Review Changes for more information). You should use these comments to ask specific questions or suggest specific changes to the PR.


Review Changes
--------------

Leaving comments does not mean that you have left a review. In addition to the comments you make, you should also explicitly mark the state of the PR. There are two ways to do this.

When you leave a code related comment, you can select the "Start a review" button, which will initiate a review. For each new comment you leave, you should click "Add review comment". When you are done, navigate to the top-right of the page, where you will find a green button that says "Finish your review".

Alternatley, you can leave all your comments first and then submit a review. Use the "Add single comment" button for each code related comment. Then, once you have finished commenting, navigate to the top-right of the page and click "Finish your review".

Upon clicking the green "Finish your review", you are presented with the ability to add additional generic comments and to update the state of the PR. If you left comments via the "Add single comment" button, then you **must** add an additional comment here to be able to submit a review. These are the three options for updating the PR:

#. Comment - Submit general feedback without explicit approval. This is ambiguous and should not be used because the developer does not know if you think changes should be made. It does not update the state of the PR.
#. Approve - Submit feedback and approve merging these changes. Use this when you you think the PR is ready to be merged into develop.
#. Request changes - Submit feedback that must be addressed before merging. Use this when you want the developer to make additional changes to the PR.


Iteration Process
-----------------


Review processes are iterative by nature, and PR reviews are no exception. A typical review process looks like this:

#. Developer submits a pull request and selects you as a reviewer.
#. You leave comments and submit a "Request change" review or an "Approve" review.
#. Developer updates the PR according to your suggestions.
#. Repeat steps 2 and 3 until you is satisfied with the PR.
#. Approve the PR when you are satisfied.

The actual amount of time it takes to perform a review or update the changes is relatively small compared to the amount of time the PR *waits* for the next step in the iteration. The wait time can be exacerbated in two ways: (1) The reviewer or developer is unaware that the PR is ready for the next step in the iteration process, and (2) the reviewer or developer is too busy with other work. To help alleviate the situation, we recommend the following guidelines for the reviewer (guidelines for the developer can be found `here <https://visit-sphinx-github-user-manual.readthedocs.io/en/develop/dev_manual/pr_create.html#iteration-process>`_)

* Immediately address the PR when you are notified that you are a reviewer. You should turn on notifications for this.
* If anything in the PR is unclear, ask specific questions using generic or code related comments. Make use of the @<username> feature to directly ping the developer of your comment.
* Clearly mark the review as "Approvd" or "Request changes".
* Ping the developer with the @<username> feature to let him know that it is ready to be updated.
* When the developer has updated the PR, make it a top priority to review it again.
* When the PR is ready to be merged into dev, approve the PR and either (1) squash-merge the PR into dev with a succinct description of the changes or (2) ping the developer to squash-merge the PR himself.

If you are chosen as a reviewer and you know that you will not be able to review the PR in a timely manner, please let the developer know and provide suggestions for who to choose instead. Once you start a PR review, you should make it a priority and stick with it until the end.




