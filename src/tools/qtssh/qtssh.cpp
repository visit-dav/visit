//
// NOTES: This file contains some functions to call from qtsshmain.c, a slightly 
//        customized version of windows/winplink.c from the PuTTY source code 
//        distribution.
// 
#include <QApplication>
#include <QInputDialog>

#include <VisItChangeUsernameWindow.h>
#include <VisItPasswordWindow.h>

#include "qtssh.h"

#include <io.h>
#include <process.h>

// Static variables.
static Config                  *qtssh_config = NULL;
static std::vector<std::string> qtssh_commandline;

// ****************************************************************************
// Function: qtssh_handle_prompt
//
// Purpose: 
//   Put up a Qt dialog to get a plink-specified value.
//
// Arguments:
//   p : The prompts for which we're gathering input.
//   i  : The index of the prompt for which we're gathering input.
//   in : The default input values.
//   inlen : The length of the default input values.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static int
qtssh_handle_prompt(prompts_t *p, int i, unsigned char *in, int inlen)
{
    int ret = -1; // unhandled value
    QInputDialog dlg;
    dlg.setWindowTitle(p->name);
    dlg.setLabelText(p->prompts[i]->prompt);
    dlg.setTextEchoMode((p->prompts[i]->echo > 0) ? QLineEdit::Normal : QLineEdit::Password);
    if(in != NULL)
    {
        char *tmp = new char[inlen+1];
        strncpy(tmp, (const char *)in, inlen);
        tmp[inlen] = '\0';
        dlg.setTextValue(tmp);
        delete [] tmp;
    }
    if(dlg.exec() == QDialog::Accepted)
    {
        QString s(dlg.textValue().simplified());
        if(!s.isEmpty())
        {
            ret = 1;
            strncpy(p->prompts[i]->result, s.toStdString().c_str(),
                    p->prompts[i]->result_len);
            p->prompts[i]->result[p->prompts[i]->result_len-1] = '\0';
        }
    }
    else
        exit(0);

    return ret;
}

// ****************************************************************************
// Function: qtssh_strdup
//
// Purpose: 
//   Duplicate a string.
//
// Arguments:
//   s : The string to duplicate.
//
// Returns:    A new string that would be freed by the user.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static char *
qtssh_strdup(const std::string &s)
{
    char *s2 = new char[s.size() + 1];
    strcpy(s2, s.c_str());
    return s2;
}

// ****************************************************************************
// Function: qtssh_handle_new_username
//
// Purpose: 
//   Called when we want to change the username that we're using to login. We
//   get the new username via VisIt's username window and then we run restart
//   qtssh with new arguments that pass a -l newuser.
//
// Arguments:
//   host : The host that we want to log into.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static void
qtssh_handle_new_username(const char *host)
{
    VisItChangeUsernameWindow win;
    VisItChangeUsernameWindow::ReturnCode status = VisItChangeUsernameWindow::UW_Rejected;
    QString username = win.getUsername(host, status);
    if(status == VisItChangeUsernameWindow::UW_Accepted && !username.isEmpty())
    {
        // Look for -l until the end or we hit an ssh command.
        bool dashL = false;
        for(size_t i = 1; i < qtssh_commandline.size(); ++i)
        {
            if(qtssh_commandline[i] == "-l")
            {
                dashL = true;
                break;
            }
            else if(qtssh_commandline[i] == "ssh")
                break;
        }

        // We need to restart this program with extra arguments to set the new username
        // on the command line.
        const char **new_argv = new const char*[qtssh_commandline.size() + (dashL ? 1 : 3)];
        int index = 0;
        new_argv[index++] = qtssh_strdup(qtssh_commandline[0]);
        // There was no -l so add one.
        if(!dashL)
        {
            new_argv[index++] = qtssh_strdup("-l");
            new_argv[index++] = qtssh_strdup(username.toStdString());
        }
        // Copy the rest of the arguments.
        for(size_t i = 1; i < qtssh_commandline.size(); ++i)
        {
            new_argv[index++] = qtssh_strdup(qtssh_commandline[i]);
            if(qtssh_commandline[i] == "-l")
            {
                new_argv[index++] = qtssh_strdup(username.toStdString());
                ++i;
            }
        }
        new_argv[index] = NULL;

#if 0
        printf("Starting: ");
        for(int i = 0; i < index; ++i)
            printf(" %s", new_argv[i]);
        printf("\n");
#endif

        // make a new console and minimize it.
        FreeConsole();
        AllocConsole();
        ShowWindow(GetConsoleWindow(), SW_MINIMIZE);

        // Start the new qtssh.exe
        _spawnvp(_P_NOWAIT, new_argv[0], new_argv);

        // Exit this one.
        cleanup_exit(0);
    }
}

// ****************************************************************************
// Function: qtssh_handle_password
//
// Purpose: 
//   Put up VisIt's password window.
//
// Arguments:
//   p : The prompts for which we're gathering input.
//   in : The default input values.
//   inlen : The length of the default input values.
//   passphrase : True if we want passphrase instead of password.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

static int
qtssh_handle_password(prompts_t *p, int i, unsigned char *in, int inlen, bool passphrase)
{
    VisItPasswordWindow win;

    // Get the new password.
getpassword:
    VisItPasswordWindow::ReturnCode status = VisItPasswordWindow::PW_Rejected;
    QString password = win.getPassword(QString(qtssh_config->username),
                                       QString(qtssh_config->host),
                                       passphrase,
                                       status);

    int ret = -1;
    if(status == VisItPasswordWindow::PW_ChangedUsername)
    {
        win.hide();

        // The user wanted to get a new username.
        qtssh_handle_new_username(qtssh_config->host);

        win.show();
        goto getpassword;
    }
    else if(status == VisItPasswordWindow::PW_Accepted)
    {
        ret = 1;

        // Copy the password back into the prompts.
        strncpy(p->prompts[i]->result, password.toStdString().c_str(),
                p->prompts[i]->result_len);
        p->prompts[i]->result[p->prompts[i]->result_len-1] = '\0';
    }
    else
    {
        // The user did not want to enter a password. Exit the program.
        exit(0);
    }

    return ret;
}

// ****************************************************************************
// Function: qtssh_get_userpass_input
//
// Purpose: 
//   We've made a slight change to plink's function that gets user passwords so
//   it calls this function first. We use this opportunity to put up out
//   password and change username windows. If we're getting some other prompt
//   then we call our own qtssh_handle_prompt function.
//
// Arguments:
//   p : The prompts for which we're gathering input.
//   in : The default input values.
//   inlen : The length of the default input values.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

int qtssh_get_userpass_input(prompts_t *p, unsigned char *in, int inlen)
{
    int ret = -1; // unhandled value

    for(int i = 0; i < p->n_prompts; ++i)
    {
        if (strstr(p->prompts[i]->prompt, "assword") != NULL)
            ret = qtssh_handle_password(p, i, in, inlen, false);
        else if (strstr(p->prompts[i]->prompt, "assphrase") != NULL)
            ret = qtssh_handle_password(p, i, in, inlen, true);
        else
            ret = qtssh_handle_prompt(p, i, in, inlen);
    }
    return ret;
}

// ****************************************************************************
// Function: qtssh_init
//
// Purpose: 
//   Init function that plink's main function calls.
//
// Arguments:
//   argc : The argument count.
//   argv : The command line arguments.
//   cfg  : The config object that holds the properties of how plink is connecting.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

int
qtssh_init(int *argc, char **argv, Config *cfg)
{
    // Stash some information we'll need.
    qtssh_config = cfg;
    for(int i = 0; i < *argc; ++i)
        qtssh_commandline.push_back(argv[i]);
    // Create a QApplication.
    new QApplication(*argc, argv);
    return 1;
}

