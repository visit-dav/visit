# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************

target_sources(viewer PRIVATE
    AnimationActionsUI.C
    LockActionsUI.C
    PlotAndOperatorActionsUI.C
    PrintWindowAction.C
    SetAppearanceAction.C
    ToolbarActionsUI.C
    ViewActionsUI.C
    ViewerActionManagerUI.C
    ViewerActionUI.C
    ViewerActionUIMultiple.C
    ViewerActionUISingle.C
    ViewerActionUIToggle.C
    ViewerChangeUsernameUI.C
    ViewerConnectionPrinterUI.C
    ViewerConnectionProgressDialog.C
    ViewerFactoryUI.C
    ViewerHostProfileSelectorWithWin.C
    ViewerMessagingUI.C
    ViewerPasswordWindow.C
    ViewerPopupMenu.C
    ViewerToolbar.C
    ViewerWindowUI.C
    VisItChangeUsernameWindow.C
    VisItPasswordWindow.C
    WindowActionsUI.C
    AnimationActionsUI.h
    ClientActionsUI.h
    LockActionsUI.h
    PlotAndOperatorActionsUI.h
    PrintWindowAction.h
    SetAppearanceAction.h
    ToolbarActionsUI.h
    ViewActionsUI.h
    ViewerActionManagerUI.h
    ViewerActionUI.h
    ViewerActionUIMultiple.h
    ViewerActionUISingle.h
    ViewerActionUIToggle.h
    ViewerChangeUsernameUI.h
    ViewerConnectionPrinterUI.h
    ViewerConnectionProgressDialog.h
    ViewerFactoryUI.h
    ViewerHostProfileSelectorWithWin.h
    ViewerMessagingUI.h
    ViewerPasswordWindow.h
    ViewerPopupMenu.h
    ViewerToolbar.h
    ViewerWindowUI.h
    VisItChangeUsernameWindow.h
    VisItPasswordWindow.h
    WindowActionsUI.h)

target_include_directories(viewer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/main/ui>)

