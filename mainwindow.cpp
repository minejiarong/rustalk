#include "mainwindow.h"
#include "rustalk_core.h"
#include "MessageListModel.h"
#include "MessageDelegate.h"
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Resize to a reasonable default
    resize(1000, 700);

    setupUi();

    applyStyles();
}

// setupWindowEffect removed as QWindowKit handles it

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // Central Widget
    centralWidget = new QWidget(this);
    centralWidget->setObjectName("CentralWidget");
    setCentralWidget(centralWidget);

    // Root stack: Login | Chat
    rootStack = new QStackedWidget(centralWidget);
    rootStack->setObjectName("RootStack");
    auto centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addWidget(rootStack);

    // ===== Login Page =====
    loginPage = new QWidget(rootStack);
    loginPage->setObjectName("LoginPage");
    loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setContentsMargins(40, 40, 40, 40);
    loginLayout->setSpacing(12);

    auto loginTitle = new QLabel("欢迎使用 Rustalk", loginPage);
    loginTitle->setObjectName("LoginTitle");
    loginTitle->setAlignment(Qt::AlignCenter);
    loginTitle->setFixedHeight(40);

    loginUsernameEdit = new QLineEdit(loginPage);
    loginUsernameEdit->setObjectName("LoginUsername");
    loginUsernameEdit->setPlaceholderText("用户名");

    loginPasswordEdit = new QLineEdit(loginPage);
    loginPasswordEdit->setObjectName("LoginPassword");
    loginPasswordEdit->setEchoMode(QLineEdit::Password);
    loginPasswordEdit->setPlaceholderText("密码");

    loginButton = new QPushButton("登录", loginPage);
    loginButton->setObjectName("LoginButton");
    loginButton->setFixedHeight(40);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);

    loginStatusLabel = new QLabel("", loginPage);
    loginStatusLabel->setObjectName("LoginStatus");
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setFixedHeight(24);

    registerButton = new QPushButton("注册", loginPage);
    registerButton->setObjectName("RegisterButton");
    registerButton->setFixedHeight(40);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::onGotoRegisterClicked);

    loginLayout->addStretch();
    loginLayout->addWidget(loginTitle);
    loginLayout->addSpacing(12);
    loginLayout->addWidget(loginUsernameEdit);
    loginLayout->addWidget(loginPasswordEdit);
    loginLayout->addWidget(loginStatusLabel);
    loginLayout->addSpacing(6);
    loginLayout->addWidget(loginButton);
    loginLayout->addWidget(registerButton);
    loginLayout->addStretch();

    // ===== Register Page =====
    registerPage = new QWidget(rootStack);
    registerPage->setObjectName("RegisterPage");
    registerLayout = new QVBoxLayout(registerPage);
    registerLayout->setContentsMargins(40, 40, 40, 40);
    registerLayout->setSpacing(12);

    auto registerTitle = new QLabel("创建账户", registerPage);
    registerTitle->setObjectName("RegisterTitle");
    registerTitle->setAlignment(Qt::AlignCenter);
    registerTitle->setFixedHeight(40);

    registerUsernameEdit = new QLineEdit(registerPage);
    registerUsernameEdit->setObjectName("RegisterUsername");
    registerUsernameEdit->setPlaceholderText("用户名");

    registerPasswordEdit = new QLineEdit(registerPage);
    registerPasswordEdit->setObjectName("RegisterPassword");
    registerPasswordEdit->setEchoMode(QLineEdit::Password);
    registerPasswordEdit->setPlaceholderText("密码");

    registerConfirmEdit = new QLineEdit(registerPage);
    registerConfirmEdit->setObjectName("RegisterConfirm");
    registerConfirmEdit->setEchoMode(QLineEdit::Password);
    registerConfirmEdit->setPlaceholderText("确认密码");

    registerStatusLabel = new QLabel("", registerPage);
    registerStatusLabel->setObjectName("RegisterStatus");
    registerStatusLabel->setAlignment(Qt::AlignCenter);
    registerStatusLabel->setFixedHeight(24);

    registerSubmitButton = new QPushButton("提交注册", registerPage);
    registerSubmitButton->setObjectName("RegisterSubmitButton");
    registerSubmitButton->setFixedHeight(40);
    connect(registerSubmitButton, &QPushButton::clicked, this, &MainWindow::onRegisterSubmitClicked);

    gotoLoginButton = new QPushButton("返回登录", registerPage);
    gotoLoginButton->setObjectName("GotoLoginButton");
    gotoLoginButton->setFixedHeight(40);
    connect(gotoLoginButton, &QPushButton::clicked, this, &MainWindow::onGotoLoginClicked);

    registerLayout->addStretch();
    registerLayout->addWidget(registerTitle);
    registerLayout->addSpacing(12);
    registerLayout->addWidget(registerUsernameEdit);
    registerLayout->addWidget(registerPasswordEdit);
    registerLayout->addWidget(registerConfirmEdit);
    registerLayout->addWidget(registerStatusLabel);
    registerLayout->addSpacing(6);
    registerLayout->addWidget(registerSubmitButton);
    registerLayout->addWidget(gotoLoginButton);
    registerLayout->addStretch();

    // ===== Chat Page =====
    chatRoot = new QWidget(rootStack);
    chatRoot->setObjectName("ChatRoot");
    mainLayout = new QHBoxLayout(chatRoot);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Sidebar ---
    sidebarContainer = new QWidget(chatRoot);
    sidebarContainer->setObjectName("Sidebar");
    sidebarContainer->setFixedWidth(280);
    
    sidebarLayout = new QVBoxLayout(sidebarContainer);
    sidebarLayout->setContentsMargins(10, 20, 10, 20);
    sidebarLayout->setSpacing(10);

    userProfileLabel = new QLabel("未登录", sidebarContainer);
    userProfileLabel->setObjectName("UserProfile");
    userProfileLabel->setAlignment(Qt::AlignCenter);
    userProfileLabel->setFixedHeight(40);

    contactList = new QListWidget(sidebarContainer);
    contactList->setObjectName("ContactList");
    contactList->setFrameShape(QFrame::NoFrame);

    groupList = new QListWidget(sidebarContainer);
    groupList->setObjectName("GroupList");
    groupList->setFrameShape(QFrame::NoFrame);

    sidebarTabs = new QTabWidget(sidebarContainer);
    sidebarTabs->setObjectName("SidebarTabs");
    sidebarTabs->addTab(contactList, "联系人");
    sidebarTabs->addTab(groupList, "群组");

    sidebarLayout->addWidget(userProfileLabel);
    sidebarLayout->addWidget(sidebarTabs);

    // --- Chat Area ---
    chatContainer = new QWidget(chatRoot);
    chatContainer->setObjectName("ChatContainer");
    
    chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    // Chat Header
    chatHeader = new QWidget(chatContainer);
    chatHeader->setObjectName("ChatHeader");
    chatHeader->setFixedHeight(60);
    
    headerLayout = new QHBoxLayout(chatHeader);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    chatTitleLabel = new QLabel("Select a contact", chatHeader);
    chatTitleLabel->setObjectName("ChatTitle");
    
    // Add minimize/close buttons (basic implementation for frameless)
    QPushButton *minBtn = new QPushButton("-", chatHeader);
    minBtn->setObjectName("TitleBtn");
    minBtn->setFixedSize(30, 30);
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);

    QPushButton *closeBtn = new QPushButton("X", chatHeader);
    closeBtn->setObjectName("TitleBtn");
    closeBtn->setFixedSize(30, 30);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    headerLayout->addWidget(chatTitleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(minBtn);
    headerLayout->addWidget(closeBtn);

    messageView = new QListView(chatContainer);
    messageView->setObjectName("MessageView");
    messageView->setFrameShape(QFrame::NoFrame);
    messageView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    messageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    messageView->setSelectionMode(QAbstractItemView::NoSelection);
    messageModel = new MessageListModel(messageView);
    messageDelegate = new MessageDelegate(messageView);
    messageView->setModel(messageModel);
    messageView->setItemDelegate(messageDelegate);

    // Input Area
    inputContainer = new QWidget(chatContainer);
    inputContainer->setObjectName("InputContainer");
    inputContainer->setFixedHeight(80);

    inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(20, 10, 20, 10);
    inputLayout->setSpacing(10);

    messageInput = new QTextEdit(inputContainer);
    messageInput->setObjectName("MessageInput");
    messageInput->setPlaceholderText("Type a message...");

    sendButton = new QPushButton("Send", inputContainer);
    sendButton->setObjectName("SendButton");
    sendButton->setFixedSize(80, 40);
    sendButton->setCursor(Qt::PointingHandCursor);
    
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    chatLayout->addWidget(chatHeader);
    chatLayout->addWidget(messageView);
    chatLayout->addWidget(inputContainer);

    // Add to main layout
    mainLayout->addWidget(sidebarContainer);
    mainLayout->addWidget(chatContainer);

    rootStack->addWidget(loginPage);
    rootStack->addWidget(registerPage);
    rootStack->addWidget(chatRoot);
    rootStack->setCurrentWidget(loginPage);

    connect(contactList, &QListWidget::itemClicked, this, &MainWindow::onContactSelected);
    connect(groupList, &QListWidget::itemClicked, this, &MainWindow::onGroupSelected);
}


void MainWindow::applyStyles()
{
    QString style = R"(
        QMainWindow { 
            background-color: #1E1E1E;
        }

        /* Global Reset */
        * {
            font-family: 'Segoe UI', sans-serif;
            font-size: 14px;
            color: #CCCCCC;
        }

        /* Login */
        #LoginPage {
            background-color: #1E1E1E;
        }
        #LoginTitle {
            font-size: 20px;
            font-weight: bold;
            color: #FFFFFF;
        }
        #LoginUsername, #LoginPassword {
            background-color: #3C3C3C;
            border: 1px solid #3C3C3C;
            border-radius: 5px;
            padding: 10px;
            color: #FFFFFF;
        }
        #LoginUsername:focus, #LoginPassword:focus {
            border: 1px solid #007ACC;
            background-color: #252526;
        }
        #LoginStatus {
            color: #CCCCCC;
        }
        #LoginButton, #RegisterButton {
            background-color: #007ACC;
            color: white;
            border: none;
            border-radius: 5px;
            font-weight: bold;
        }
        #LoginButton:hover, #RegisterButton:hover {
            background-color: #0062A3;
        }

        /* Register */
        #RegisterPage {
            background-color: #1E1E1E;
        }
        #RegisterTitle {
            font-size: 20px;
            font-weight: bold;
            color: #FFFFFF;
        }
        #RegisterUsername, #RegisterPassword, #RegisterConfirm {
            background-color: #3C3C3C;
            border: 1px solid #3C3C3C;
            border-radius: 5px;
            padding: 10px;
            color: #FFFFFF;
        }
        #RegisterUsername:focus, #RegisterPassword:focus, #RegisterConfirm:focus {
            border: 1px solid #007ACC;
            background-color: #252526;
        }
        #RegisterStatus {
            color: #CCCCCC;
        }
        #RegisterSubmitButton, #GotoLoginButton {
            background-color: #007ACC;
            color: white;
            border: none;
            border-radius: 5px;
            font-weight: bold;
        }
        #RegisterSubmitButton:hover, #GotoLoginButton:hover {
            background-color: #0062A3;
        }

        /* Central Widget */
        #CentralWidget {
            background-color: #1E1E1E;
        }

        /* Sidebar */
        #Sidebar {
            background-color: #252526;
            border-right: 1px solid #2B2B2B;
        }

        #UserProfile {
            font-size: 16px;
            font-weight: bold;
            color: #FFFFFF;
            padding-bottom: 10px;
            border-bottom: 1px solid #2B2B2B;
        }

        /* Contact List */
        #ContactList {
            background-color: transparent;
            outline: none;
            border: none;
        }
        #ContactList::item {
            height: 50px;
            padding: 5px;
            border-radius: 5px;
            margin: 2px 5px;
            color: #CCCCCC;
        }
        #ContactList::item:selected {
            background-color: #37373D;
            color: #FFFFFF;
        }
        #ContactList::item:hover {
            background-color: #2A2D2E;
        }

        /* Chat Container */
        #ChatContainer {
            background-color: #1E1E1E;
        }

        /* Chat Header */
        #ChatHeader {
            background-color: #1E1E1E;
            border-bottom: 1px solid #2B2B2B;
        }

        #ChatTitle {
            font-size: 18px;
            font-weight: bold;
            color: #FFFFFF;
        }

        #TitleBtn {
            background-color: transparent;
            color: #CCCCCC;
            border: none;
            font-weight: bold;
        }
        #TitleBtn:hover {
            color: #FFFFFF;
            background-color: #333333;
            border-radius: 5px;
        }

        /* Message View */
        #MessageView {
            background-color: transparent;
            outline: none;
            padding: 10px;
            border: none;
        }

        /* Input Area */
        #InputContainer {
            background-color: #1E1E1E;
            border-top: 1px solid #2B2B2B;
        }

        #MessageInput {
            background-color: #3C3C3C;
            border: 1px solid #3C3C3C;
            border-radius: 5px;
            padding: 10px;
            color: #FFFFFF;
            selection-background-color: #007ACC;
        }
        #MessageInput:focus {
            border: 1px solid #007ACC;
            background-color: #252526;
        }

        #SendButton {
            background-color: #007ACC;
            color: white;
            border: none;
            border-radius: 5px;
            font-weight: bold;
            padding: 5px 15px;
        }
        #SendButton:hover {
            background-color: #0062A3;
        }
        #SendButton:pressed {
            background-color: #005A9E;
        }
        
        /* Scrollbars */
        QScrollBar:vertical {
            border: none;
            background: #1E1E1E;
            width: 10px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #424242;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #686868;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
        }
    )";
    this->setStyleSheet(style);
}

// Frameless Window Logic
// (Removed manual implementation as FramelessHelper handles it)

void MainWindow::onContactSelected(QListWidgetItem *item)
{
    if (!item) return;
    chatTitleLabel->setText(item->text());
    currentPeerId = item->data(Qt::UserRole).toLongLong();
    currentIsGroup = false;
    messageModel->setCurrentUserId(currentUserId);
    messageModel->setCurrentPeer(currentPeerId, false);
    messageModel->loadHistory(100);
}

void MainWindow::onSendMessage()
{
    QString text = messageInput->toPlainText().trimmed();
    if (text.isEmpty()) return;

    if (currentPeerId == 0) return;
    rustalk_send_message(currentUserId, currentPeerId, text.toUtf8().constData());
    qint64 ts = QDateTime::currentSecsSinceEpoch();
    messageModel->appendMessage(currentUserId, currentPeerId, text, ts);
    messageInput->clear();
}

void MainWindow::onLoginClicked()
{
    QString uname = loginUsernameEdit->text().trimmed();
    QString pwd = loginPasswordEdit->text();
    if (uname.isEmpty() || pwd.isEmpty()) {
        loginStatusLabel->setText("请输入用户名与密码");
        loginStatusLabel->setStyleSheet("color:#E81123;");
        return;
    }
    loginButton->setEnabled(false);
    qint64 userId = 0;
    int rc = rustalk_login(uname.toUtf8().constData(), pwd.toUtf8().constData(), &userId);
    if (rc == 0 && userId > 0) {
        loginStatusLabel->setText("");
        userProfileLabel->setText(uname);
        currentUserId = userId;
        messageModel->setCurrentUserId(currentUserId);
        rootStack->setCurrentWidget(chatRoot);
        rustalk_connect("ws://localhost:8080");

        int len = 0;
        ContactFFI* contacts = rustalk_fetch_contacts(&len);
        contactList->clear();
        if (contacts && len > 0) {
            for (int i = 0; i < len; i++) {
                QString name = QString::fromUtf8(contacts[i].name);
                QListWidgetItem* item = new QListWidgetItem(name, contactList);
                item->setData(Qt::UserRole, (qlonglong)contacts[i].id);
            }
            rustalk_free_contacts(contacts, len);
        } else {
            new QListWidgetItem("No Contacts", contactList);
        }
        groupList->clear();
        {
            QListWidgetItem *g1 = new QListWidgetItem("开发群", groupList);
            g1->setData(Qt::UserRole, (qlonglong)1001);
            QListWidgetItem *g2 = new QListWidgetItem("产品群", groupList);
            g2->setData(Qt::UserRole, (qlonglong)1002);
        }
        if (contactList->count() > 0) {
            contactList->setCurrentRow(0);
            onContactSelected(contactList->item(0));
        }
    } else {
        if (rc == -20) {
            loginStatusLabel->setText("密码错误");
        } else if (rc == -21) {
            loginStatusLabel->setText("用户不存在");
        } else {
            loginStatusLabel->setText("登录失败");
        }
        loginStatusLabel->setStyleSheet("color:#E81123;");
    }
    loginButton->setEnabled(true);
}

void MainWindow::onRegisterClicked()
{
    rootStack->setCurrentWidget(registerPage);
}

void MainWindow::onGotoRegisterClicked()
{
    rootStack->setCurrentWidget(registerPage);
}

void MainWindow::onGotoLoginClicked()
{
    rootStack->setCurrentWidget(loginPage);
}

void MainWindow::onRegisterSubmitClicked()
{
    QString uname = registerUsernameEdit->text().trimmed();
    QString pwd = registerPasswordEdit->text();
    QString confirm = registerConfirmEdit->text();
    if (uname.isEmpty() || pwd.isEmpty() || confirm.isEmpty()) {
        registerStatusLabel->setText("请完整填写信息");
        registerStatusLabel->setStyleSheet("color:#E81123;");
        return;
    }
    if (pwd != confirm) {
        registerStatusLabel->setText("两次输入的密码不一致");
        registerStatusLabel->setStyleSheet("color:#E81123;");
        return;
    }
    registerSubmitButton->setEnabled(false);
    qint64 userId = 0;
    int rc = rustalk_register(uname.toUtf8().constData(), pwd.toUtf8().constData(), &userId);
    if (rc == 0 && userId > 0) {
        registerStatusLabel->setText("注册成功，请登录");
        registerStatusLabel->setStyleSheet("color:#22C55E;");
        loginUsernameEdit->setText(uname);
        rootStack->setCurrentWidget(loginPage);
    } else {
        if (rc == -10) {
            registerStatusLabel->setText("用户名已存在");
        } else {
            registerStatusLabel->setText("注册失败");
        }
        registerStatusLabel->setStyleSheet("color:#E81123;");
    }
    registerSubmitButton->setEnabled(true);
}

void MainWindow::onGroupSelected(QListWidgetItem *item)
{
    if (!item) return;
    chatTitleLabel->setText(item->text());
    currentPeerId = item->data(Qt::UserRole).toLongLong();
    currentIsGroup = true;
    messageModel->setCurrentUserId(currentUserId);
    messageModel->setCurrentPeer(currentPeerId, true);
    messageModel->loadHistory(100);
}
