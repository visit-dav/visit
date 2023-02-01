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

#if defined(_WIN32)
#include <io.h>
#include <process.h>
#endif

// Static variables.
static Conf                  *qtssh_config = NULL;
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
//     Kevin Griffin, Fri Jan  6 13:39:12 PST 2017
//     Changed p->prompts[i]->result_len to p->prompts[i]->resultsize
//     due to the putty upgrade. Also checked for resultsize being zero before
//     copying to it.
//   
// ****************************************************************************

static int
qtssh_handle_prompt(prompts_t *p, int i, unsigned char *in, int inlen)
{
    int ret = -1; // unhandled value
    QInputDialog dlg;
    dlg.setWindowTitle(p->name);
    dlg.setLabelText(p->prompts[i]->prompt);
    dlg.setTextEchoMode((p->prompts[i]->echo) ? QLineEdit::Normal : QLineEdit::Password);
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
            
            prompt_set_result(p->prompts[i], s.toStdString().c_str());
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
//   Cyrus Harrison, Wed Apr 17 12:34:05 PDT 2019
//   Switched to term string with '\0' instead of NULL, to avoid a compiler
//   warning.
// ****************************************************************************

static char *
qtssh_strdup(const std::string &s)
{
    char *s2 = new char[s.size() + 1];
    strcpy(s2, s.c_str());
    return s2;
}

static char *
qtssh_strdup_with_quotes(const std::string &s)
{

    if (std::count(s.begin(), s.end(), '\\') > 0)
    {
        char *s2 = new char[s.size() + 3];
        s2[0] = '\"';
        strcpy(&s2[1], s.c_str());
        s2[s.size()+1] = '\"';
        s2[s.size()+2] = '\0';
        return s2;
    }
    return qtssh_strdup(s);
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
//   Kevin Griffin, Thu Jan 12 14:29:26 PST 2017
//   Used strcmp instead of ==, set CONF_username to the new username
//
//   Kathleen Biagas, Tue May 22, 2018
//   Removed WIN32 code that Freed and Alloc'd new console.
//
//   Kathleen Biagas, Tue Aug 21, 2018
//   Re-added WIN32 code that Freed and Alloc'd new console.  Also, surround
//   new_argv[0] with quotes on Windows, required when qtssh command is full
//   path.  But 'cmd' arg to _spanwvp requires no quotes, so use
//   qtssh_commandline[0].
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
        conf_set_str(qtssh_config, CONF_username, dupstr(username.toStdString().c_str()));
        
        // Look for -l until the end or we hit an ssh command.
        bool dashL = false;
        for(size_t i = 1; i < qtssh_commandline.size(); ++i)
        {
            if(strcmp(qtssh_commandline[i].c_str(),"-l") == 0)
            {
                dashL = true;
                break;
            }
            else if(strcmp(qtssh_commandline[i].c_str(),"ssh") == 0)
                break;
        }

        // We need to restart this program with extra arguments to set the
        // new username on the command line.
        const char **new_argv = new const char*[qtssh_commandline.size() + (dashL ? 1 : 3)];
        int index = 0;
#ifdef _WIN32
        // surround the qtssh command with quotes if necessary
        new_argv[index++] = qtssh_strdup_with_quotes(qtssh_commandline[0]);
#else
        new_argv[index++] = qtssh_strdup(qtssh_commandline[0]);
#endif
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
            if(strcmp(qtssh_commandline[i].c_str(),"-l") == 0)
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

#if defined(_WIN32)
        // make a new console and minimize it.
        FreeConsole();
        AllocConsole();
        ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
        // Start the new qtssh.exe, using the non-quoted qtssh command
        _spawnvp(_P_NOWAIT, qtssh_commandline[0].c_str(), new_argv);
        // Exit this one.
        cleanup_exit(0);
#endif
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
//   phrase : Phrase to use in the prompt
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 14:02:02 PDT 2012
//
// Modifications:
//     Kevin Griffin, Fri Jan  6 13:39:12 PST 2017
//     Changed p->prompts[i]->result_len to p->prompts[i]->resultsize
//     due to the putty upgrade. Added check for resultsize being equal to
//     zero. Changed struct to new conf_get_* format.
//   
// ****************************************************************************

static int
qtssh_handle_password(prompts_t *p, int i, unsigned char *in, int inlen, std::string phrase)
{
    VisItPasswordWindow win;
    
    // Get the new password.
getpassword:
    VisItPasswordWindow::ReturnCode status = VisItPasswordWindow::PW_Rejected;
    QString password = win.getPassword(QString(conf_get_str(qtssh_config, CONF_username)),
                                       QString(conf_get_str(qtssh_config, CONF_host)),
                                       QString(phrase.c_str()),
                                       status);
    
    int ret = -1;
    if(status == VisItPasswordWindow::PW_ChangedUsername)
    {
        win.hide();
        
        // The user wanted to get a new username.
        qtssh_handle_new_username(conf_get_str(qtssh_config, CONF_host));
        
        win.show();
        goto getpassword;
    }
    else if(status == VisItPasswordWindow::PW_Accepted)
    {
        ret = 1;
        prompt_set_result(p->prompts[i], password.toStdString().c_str());
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

    for(size_t i = 0; i < p->n_prompts; ++i)
    {
        if (strstr(p->prompts[i]->prompt, "assword") != NULL)
            ret = qtssh_handle_password(p, i, in, inlen, "Password");
        else if (strstr(p->prompts[i]->prompt, "asscode") != NULL)
            ret = qtssh_handle_password(p, i, in, inlen, "Passcode");
        else if (strstr(p->prompts[i]->prompt, "assphrase") != NULL)
            ret = qtssh_handle_password(p, i, in, inlen, "Passphrase");
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
qtssh_init(int *argc, char **argv, Conf *cfg)
{
    // Stash some information we'll need.
    qtssh_config = cfg;
    for(int i = 0; i < *argc; ++i)
        qtssh_commandline.push_back(argv[i]);
    // Create a QApplication.
    new QApplication(*argc, argv);
    return 1;
}

