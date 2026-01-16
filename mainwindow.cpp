#include "mainwindow.h"
#include "rustalk_core.h"
#include "MessageListModel.h"
#include "MessageDelegate.h"
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>
#include <QListWidgetItem>
#include <QFontDatabase>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QFileDialog>
#include <QTextStream>
#include <QWidgetAction>
#include <QGridLayout>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Load Custom Font
    int fontId = QFontDatabase::addApplicationFont("D:/hw/qt/RUSTALK/华康宋体W12.ttf");
    QString fontFamily = "Segoe UI"; // Fallback
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            fontFamily = families.first();
        }
    }
    // Set default font
    qApp->setFont(QFont(fontFamily, 12));
    this->setProperty("appFont", fontFamily);

    // Resize to a reasonable default
    resize(1000, 700);

    setupUi();

    applyStyles();
    startWsServer();
}

// setupWindowEffect removed as QWindowKit handles it

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

MainWindow::~MainWindow()
{
    if (wsServerProc) {
        wsServerProc->kill();
        wsServerProc->waitForFinished(2000);
        wsServerProc = nullptr;
    }
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
    contactList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(contactList, &QListWidget::customContextMenuRequested, this, &MainWindow::onContactContextMenu);

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
    
    connectionStatusLabel = new QLabel("未连接", chatHeader);
    connectionStatusLabel->setObjectName("ConnectionStatus");
    connectionStatusLabel->setContentsMargins(10, 0, 0, 0);

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
    headerLayout->addWidget(connectionStatusLabel);

    searchEdit = new QLineEdit(chatHeader);
    searchEdit->setPlaceholderText("搜索...");
    searchEdit->setFixedWidth(150);
    searchEdit->setStyleSheet("QLineEdit { background-color: #333333; color: white; border: 1px solid #444; border-radius: 4px; padding: 4px; }");
    connect(searchEdit, &QLineEdit::returnPressed, [this]() {
        if (messageModel) {
            messageModel->searchMessages(searchEdit->text());
        }
        if (messageDelegate) {
            messageDelegate->setSearchKeyword(searchEdit->text());
            messageView->viewport()->update();
        }
    });
    connect(searchEdit, &QLineEdit::textChanged, [this](const QString& t) {
        if (messageDelegate) {
            messageDelegate->setSearchKeyword(t);
            messageView->viewport()->update();
        }
    });
    headerLayout->addWidget(searchEdit);

    headerLayout->addStretch();
    headerLayout->addWidget(minBtn);
    headerLayout->addWidget(closeBtn);

    messageView = new QListView(chatContainer);
    messageView->setObjectName("MessageView");
    messageView->setFrameShape(QFrame::NoFrame);
    messageView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    messageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    messageView->setSelectionMode(QAbstractItemView::NoSelection);
    messageView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(messageView, &QListView::customContextMenuRequested, this, &MainWindow::onMessageContextMenu);
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

    emojiButton = new QPushButton(QString::fromUtf8("😊"), inputContainer);
    emojiButton->setObjectName("EmojiButton");
    emojiButton->setFixedSize(40, 40);
    connect(emojiButton, &QPushButton::clicked, [this]() {
        buildEmojiMenu();
        if (emojiMenu) {
            QPoint p = emojiButton->mapToGlobal(QPoint(0, emojiButton->height()));
            emojiMenu->popup(p);
        }
    });

    sendButton = new QPushButton("Send", inputContainer);
    sendButton->setObjectName("SendButton");
    sendButton->setFixedSize(80, 40);
    sendButton->setCursor(Qt::PointingHandCursor);
    
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(emojiButton);
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
    QString fontFamily = this->property("appFont").toString();
    if (fontFamily.isEmpty()) fontFamily = "Segoe UI";

    QString style = QString(R"(
        QMainWindow { 
            background-color: #1E1E1E;
        }

        /* Global Reset */
        * {
            font-family: '%1', sans-serif;
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

        /* Sidebar Tabs */
        QTabWidget::pane {
            border: none;
            background: #252526;
        }
        QTabWidget::tab-bar {
            alignment: left;
        }
        QTabBar::tab {
            background: #252526;
            color: #CCCCCC;
            padding: 8px 16px;
            border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected {
            color: #FFFFFF;
            border-bottom: 2px solid #007ACC;
        }
        QTabBar::tab:hover {
            background: #2D2D30;
        }

        /* Contact & Group List */
        QListWidget {
            background-color: #252526;
            outline: none;
            border: none;
        }
        QListWidget::item {
            height: 40px;
            padding: 5px;
            border-radius: 4px;
            margin: 2px 5px;
            color: #CCCCCC;
        }
        QListWidget::item:selected {
            background-color: #37373D;
            color: #FFFFFF;
        }
        QListWidget::item:hover {
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

        #ConnectionStatus {
            font-size: 12px;
            color: #888888;
            background-color: #333333;
            padding: 2px 8px;
            border-radius: 10px;
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
    )").arg(fontFamily);
    this->setStyleSheet(style);
}

// Frameless Window Logic
// (Removed manual implementation as FramelessHelper handles it)

void MainWindow::startWsServer()
{
    if (wsServerProc) return;
    QTcpSocket sock;
    sock.connectToHost("127.0.0.1", 8080);
    if (sock.waitForConnected(100)) {
        sock.abort();
        return;
    }
    wsServerProc = new QProcess(this);
    QString exe = QStringLiteral(RUSTALK_SERVER_DIR) + "/target/debug/rustakl_sever.exe";
    if (QFile::exists(exe)) {
        wsServerProc->setProgram(exe);
        wsServerProc->start();
    } else {
        wsServerProc->setWorkingDirectory(QStringLiteral(RUSTALK_SERVER_DIR));
        wsServerProc->setProgram("cargo");
        wsServerProc->setArguments(QStringList() << "run");
        wsServerProc->start();
    }
}

void MainWindow::onMessageContextMenu(const QPoint &pos)
{
    QModelIndex index = messageView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    QAction *deleteAction = menu.addAction("删除");
    connect(deleteAction, &QAction::triggered, [this, index]() {
        messageModel->deleteMessage(index.row());
    });
    QAction *copyAction = menu.addAction("复制到剪贴板");
    connect(copyAction, &QAction::triggered, [this, index]() {
        copyToClipboard(index);
    });
    QAction *exportAction = menu.addAction("导出到TXT");
    connect(exportAction, &QAction::triggered, [this]() {
        exportToTXT();
    });
    menu.exec(messageView->viewport()->mapToGlobal(pos));
}

void MainWindow::copyToClipboard(const QModelIndex &index)
{
    QString content = index.data(MessageListModel::ContentRole).toString();
    QClipboard *cb = QApplication::clipboard();
    if (cb) cb->setText(content);
}

void MainWindow::exportToTXT()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出到TXT", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    const int rows = messageModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex idx = messageModel->index(i, 0);
        if (!idx.isValid()) continue;
        bool isDivider = idx.data(MessageListModel::IsDividerRole).toBool();
        if (isDivider) continue;
        QString content = idx.data(MessageListModel::ContentRole).toString();
        out << content << "\n";
    }
    file.close();
}

void MainWindow::buildEmojiMenu()
{
    if (emojiMenu) return;
    emojiMenu = new QMenu(emojiButton);
    QWidget *panel = new QWidget(emojiMenu);
    QGridLayout *grid = new QGridLayout(panel);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setSpacing(6);
    QStringList emojis = {
        "😀","😁","😂","🤣","😊","😇","🙂","🙃",
        "😉","😌","😍","🥰","😘","😗","😙","😚",
        "😜","😝","😛","🤗","🤔","🤭","🤫","🤐",
        "😶","😏","😒","🙄","😞","😔","😕","🙁",
        "☹️","😣","😖","😫","😤","😡","😠","😱",
        "😳","🥵","🥶","🥴","🤒","🤕","🤧","🤢",
        "🤮","🤯","😴","🥱","🤤","🫠","🫡","🫣"
    };
    int cols = 8;
    for (int i = 0; i < emojis.size(); ++i) {
        QPushButton *b = new QPushButton(emojis[i], panel);
        b->setFixedSize(32, 32);
        connect(b, &QPushButton::clicked, [this, i, emojis]() {
            insertEmoji(emojis[i]);
            if (emojiMenu) emojiMenu->hide();
        });
        grid->addWidget(b, i / cols, i % cols);
    }
    QWidgetAction *wa = new QWidgetAction(emojiMenu);
    wa->setDefaultWidget(panel);
    emojiMenu->addAction(wa);
}

void MainWindow::insertEmoji(const QString& emoji)
{
    QTextCursor cursor = messageInput->textCursor();
    cursor.insertText(emoji);
    messageInput->setTextCursor(cursor);
    messageInput->setFocus();
}

void MainWindow::onContactContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = contactList->itemAt(pos);
    if (!item) return;
    QMenu menu(this);
    QAction *editRemark = menu.addAction("编辑备注");
    QAction *togglePin = nullptr;
    bool pinned = item->data(Qt::UserRole + 2).toBool();
    if (pinned) {
        togglePin = menu.addAction("取消置顶");
    } else {
        togglePin = menu.addAction("置顶");
    }
    QAction *sel = menu.exec(contactList->viewport()->mapToGlobal(pos));
    if (!sel) return;
    if (sel == editRemark) {
        QString baseName = item->data(Qt::UserRole + 3).toString();
        QString oldRemark = item->data(Qt::UserRole + 1).toString();
        QString remark = QInputDialog::getText(this, "编辑备注", "备注：", QLineEdit::Normal, oldRemark);
        if (!remark.isNull()) {
            item->setData(Qt::UserRole + 1, remark);
            if (remark.trimmed().isEmpty()) {
                item->setText(baseName);
            } else {
                item->setText(baseName + " - " + remark.trimmed());
            }
        }
    } else if (sel == togglePin) {
        bool cur = item->data(Qt::UserRole + 2).toBool();
        item->setData(Qt::UserRole + 2, !cur);
        refreshContactListOrder();
    }
}

void MainWindow::refreshContactListOrder()
{
    QList<QListWidgetItem*> pinned;
    QList<QListWidgetItem*> normal;
    while (contactList->count() > 0) {
        QListWidgetItem *it = contactList->takeItem(0);
        if (!it) break;
        if (it->data(Qt::UserRole + 2).toBool()) pinned.push_back(it);
        else normal.push_back(it);
    }
    for (auto *it : pinned) {
        contactList->addItem(it);
    }
    for (auto *it : normal) {
        contactList->addItem(it);
    }
}

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
    qint64 ts = QDateTime::currentMSecsSinceEpoch();
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
        
        connectionStatusLabel->setText("正在连接...");
        connectionStatusLabel->setStyleSheet("color: #FFA500;"); // Orange

        if (rustalk_connect("ws://localhost:8080") == 0) {
            connectionStatusLabel->setText("已连接");
            connectionStatusLabel->setStyleSheet("color: #22C55E;"); // Green
        } else {
            connectionStatusLabel->setText("连接失败");
            connectionStatusLabel->setStyleSheet("color: #E81123;"); // Red
        }

        int len = 0;
        ContactFFI* contacts = rustalk_fetch_contacts(&len);
        contactList->clear();
        struct TempContact { qint64 id; QString name; };
        QList<TempContact> mockContacts = {
            {101, "碇真嗣 (Shinji)"},
            {102, "绫波丽 (Rei)"},
            {103, "式波·明日香 (Asuka)"},
            {104, "渚薰 (Kaworu)"},
            {105, "鹿目圆 (Madoka)"},
            {106, "晓美焰 (Homura)"},
            {107, "巴麻美 (Mami)"},
            {108, "美树沙耶香 (Sayaka)"},
            {109, "佐仓杏子 (Kyoko)"}
        };

        for (const auto& c : mockContacts) {
            QListWidgetItem* item = new QListWidgetItem(c.name, contactList);
            item->setData(Qt::UserRole, (qlonglong)c.id);
            item->setData(Qt::UserRole + 1, QString());
            item->setData(Qt::UserRole + 2, false);
            item->setData(Qt::UserRole + 3, c.name);
            item->setText(c.name);
        }

        groupList->clear();
        {
            QListWidgetItem *g1 = new QListWidgetItem("NERV 指挥部", groupList);
            g1->setData(Qt::UserRole, (qlonglong)2001);
            QListWidgetItem *g2 = new QListWidgetItem("见泷原魔法少女", groupList);
            g2->setData(Qt::UserRole, (qlonglong)2002);
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
