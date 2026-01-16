#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QTabWidget>
#include <QProcess>
#include <QFile>
#include <QTcpSocket>

class QListWidgetItem;
class QModelIndex;
class QMenu;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onContactSelected(QListWidgetItem *item);
    void onGroupSelected(QListWidgetItem *item);
    void onSendMessage();
    void onLoginClicked();
    void onRegisterClicked();
    void onGotoRegisterClicked();
    void onGotoLoginClicked();
    void onRegisterSubmitClicked();
    void onMessageContextMenu(const QPoint &pos);
    void onContactContextMenu(const QPoint &pos);

private:
    void setupUi();
    void applyStyles();
    void startWsServer();
    void copyToClipboard(const QModelIndex &index);
    void exportToTXT();
    void buildEmojiMenu();
    void insertEmoji(const QString& emoji);
    void refreshContactListOrder();

    // Core UI components
    QWidget *centralWidget;
    QStackedWidget *rootStack;
    QWidget *chatRoot;
    QHBoxLayout *mainLayout;

    // Sidebar
    QWidget *sidebarContainer;
    QVBoxLayout *sidebarLayout;
    QLabel *userProfileLabel;
    QTabWidget *sidebarTabs;
    QListWidget *contactList;
    QListWidget *groupList;

    // Login
    QWidget *loginPage;
    QVBoxLayout *loginLayout;
    QLineEdit *loginUsernameEdit;
    QLineEdit *loginPasswordEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QLabel *loginStatusLabel;

    // Register
    QWidget *registerPage;
    QVBoxLayout *registerLayout;
    QLineEdit *registerUsernameEdit;
    QLineEdit *registerPasswordEdit;
    QLineEdit *registerConfirmEdit;
    QPushButton *registerSubmitButton;
    QPushButton *gotoLoginButton;
    QLabel *registerStatusLabel;

    // Chat Area
    QWidget *chatContainer;
    QVBoxLayout *chatLayout;
    
    // Chat Header
    QWidget *chatHeader;
    QHBoxLayout *headerLayout;
    QLabel *chatTitleLabel;
    QLabel *connectionStatusLabel;
    QLineEdit *searchEdit;

    // Messages
    QListView *messageView;
    class MessageListModel *messageModel;
    class MessageDelegate *messageDelegate;

    // Input Area
    QWidget *inputContainer;
    QHBoxLayout *inputLayout;
    QTextEdit *messageInput;
    QPushButton *sendButton;
    QPushButton *emojiButton;
    QMenu *emojiMenu{nullptr};

    qint64 currentUserId{0};
    qint64 currentPeerId{0};
    bool currentIsGroup{false};
    QProcess *wsServerProc{nullptr};
};
#endif // MAINWINDOW_H
