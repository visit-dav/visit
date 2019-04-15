Creating a Pull Request
=======================

Overview
--------

The Pull Request (PR) is a mechanism that allows other developers on the team to review your work before merging your branch into develop. It is extremely useful for preventing bugs, enforcing coding practicing, and ensuring changes are consistent with VisIt's overall vision. Although a review does slow down the developmenet process, there are several policies we put into place to ensure a review is completed in a timely manner, and the benefits of a review outweight the additional development time.


Working with the Template
-------------------------

PR's come pre-populated with a template to help guide the content. You do not have to use this template, but keep in mind that a review needs structured context in order to accurately and quickly review your PR, so it is best to use the template or something very similar to it. The text in the template is designed to be **replaced** by your information. For example, you should replace the line that says "Please include a summary of the change" with the actual summary of the change.

In general, if there is part of the template you will not use, please delete it before submitting the PR. For example, delete any items in the checklist that are not relevant to your pull request (see Checklist below).

If you plan to add additional structured units to the template, please use markdown styling.


Description
~~~~~~~~~~~

By using the '#<number>' feature, we can link issues and PR's to one another. For example, when we type '#' followed by a number, a search menu will appear providing potential matches. Once we have selected the correct number and submitted the ticket/PR, the items will be linked. In addition, GitHub provides several keywords that update Issues based on the given state of the related PR, such as "Resolves". When a PR and ticket are linked via "Resolves", then the ticket will automatically close when the branch is merged into develop. We use these feature to keep our tickets and PR's organized, and our template sets it up for you. All you have to do is start typing the number next to the '#'.

If your PR is unrelated to a ticket, please delete the "Resolves #..." line for clarity.

Replace the line "Please include a summary of the change" with the actual summary of the change.

Type of Change
~~~~~~~~~~~~~~

Bug fixes, features, and documentation improvements are among the most common types of PR's. You may select from the menu by replacing the space between the '[ ]' with a capital X, so that it looks exactly like '[X]'. You can also make the selection *after* submitting the PR by checking the box that appears.

If you check "Other", please describe the type below the checklist.

Testing
~~~~~~~

Replace the content of this section with a description of how you tested the change.


The Checklist
~~~~~~~~~~~~~

The Checklist serves as a list of tasks you should do before submitting the PR, and you should check them off before doing them. Any items that do not relate to your PR should be erased (for example, if you did not submit a bugfix or feature, you probably won't need to add an additional test; this line in the checklist should be erased).


Notes
~~~~~

The template ends with two notes. The first is a note to squash merge the PR when it is ready to be merged. This allows you to turn several commits for one branch into one descriptive commit that encapsulates the work done on the branch. This keeps our git history cleaner and shorter. The second is a note to wait for the CI testing to finish before merging the PR. This is crucial because *if a CI test fails, then the branch should not be merged*! CI tests take approximately 30 minutes to complete.


Reviewers
---------

If you are not an owner, then GitHub will not allow your PR to be merged into develop without a reviewer's approval, so you will need at least one reviewer. If you are an owner, then you can merge the PR into develop without review, but that doesn't necessarily mean you should. Follow these guidelines to determine the need and number of reviewers. Note, these guidelines serve as a "lower bound" of the need and number for reviewers. You may always add more reviewers to your PR if you feel that is necessary.


No Reviewers (owners only)
~~~~~~~~~~~~~~~~~~~~~~~~~~

If your changes are localized and you have satsified all the testing requirements and you are confident in the correctness of your changes (where correctness is measured by both the correctness of your code for accomplishing the desired task and the correctness of *how* you implemented the code according to VisIt's policies) then you may merge the PR without a reviewer *after* the CI tests pass.


One reviewer
~~~~~~~~~~~~

If your changes have a broader impact or you are working in an area of VisIt with which you are not familiar or you are changing existing behavior, then you should add a reviewer.

If you are not an owner, then you must always have at least one reviewer, even if you satisfy the guidelines for the "No Reviewers" case. 


Two or more reviewers
~~~~~~~~~~~~~~~~~~~~~

If your changes substantially modify existing behavior or you are updating significant amounts of the code or you are designing new architectures or interfacess, then you should have at least two reviewers.


Choosing Reviewers
~~~~~~~~~~~~~~~~~~

GitHub automatically suggests reviewers based on the blame data for the files you have modified. So unless you have a specific need for a specific reviewer, then you're probably safe to go with GitHub's suggestions.


Iteration Process
-----------------

Review processes are iterative by nature, and PR reviews are no exception. A typical review process looks like this:

#. You submit a pull request and select a reviewer.
#. Reviewer goes through and leave comments, suggestions, and tasks for you to do.
#. You will get clarification for anything that us unclear and update your PR according to the suggestions.
#. Repeat steps 2 and 3 until reviewer is satisfied with the PR.
#. The reviewer approves the PR when he is satisifed.

The actual amount of time it takes to perform a review or update the changes is relatively small compared to the amount of time the PR *waits* for the next step in the iteration. The wait time can be exacerbated in two ways: (1) The reviewer or developer is unaware that the PR is ready for the next step in the iteration process, and (2) the reviewer or developer is too busy with other work. To help alleviate the situation, we recommend the following guidelines for the developer (guidelines for the reviewer can be found AT THIS PAGE WHICH I STILL NEED TO CREATE AND LINK TO).

* Make sure your code is clear and well commented and that your PR is descriptive. This helps your reviewer quickly familiarize himself with the context of your changes. If code is unclear, the reviewer may spend a lot of time trying to grasp the purpose and effects of your changes.
* Immediately answer any questions your reviewer asks about the PR. You should turn on notifications for the repo.
* When the reviewer has finished reviewing (step 2), make it a top priority to update your code according to the requested changes. Use the @<username> feature to notifiy the reviewer of any questions you have about the suggestions/comments.
* When you have finished updating your PR (step 3), leave a comment on the PR using @<username> to let the reviewer know that the PR is ready to be looked at again.
* Reviewers won't always merge a PR after they approve it, especially if the squash-merge involves combining many commits into a single, descriptive comment. When your PR is approved, you should squash-merge to develop with a succinct description. (CHECK IF NON-OWNERS CAN MERGE INTO DEV).



