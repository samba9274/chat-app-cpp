#include <ncurses.h>
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include <sio_client.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;
using nlohmann::json;
using namespace sio;

#define URL "http://samba9274.duckdns.org:3000"

class Application
{
private:
    static Application *instance;

    const string username;

    int BLACK_ON_BLACK;
    int RED_ON_BLACK;
    int GREEN_ON_BLACK;
    int YELLOW_ON_BLACK;
    int BLUE_ON_BLACK;
    int MAGENTA_ON_BLACK;
    int CYAN_ON_BLACK;
    int WHITE_ON_BLACK;

    enum Window
    {
        USERS,
        MESSAGES,
        CHAT
    };

    class Message
    {
    private:
        string username;
        string message;

    public:
        Message(string username, string message) : username(username), message(message) {}
        string getUsername() { return this->username; }
        string getMessage() { return this->message; }
        json getJson() { return json({{"username", this->username}, {"message", this->message}}); }
    };

    WINDOW *winUsersBorder;
    WINDOW *winUsers;
    WINDOW *winMessagesBorder;
    WINDOW *winMessages;
    WINDOW *winChatBorder;
    WINDOW *winChat;

    Window selectedWindow;

    string message;
    set<string> users;
    vector<Message> messages;

    int messageScroll;

    sio::client io;
    socket::ptr current_socket;

    void bind_events()
    {
        current_socket->on("chat", sio::socket::event_listener_aux(
                                       [&](string const &name, message::ptr const &data, bool isAck, message::list &ack_resp)
                                       {
                                           json j = json::parse(data->get_string());
                                           messages.push_back(Message(j["username"], j["message"]));
                                           refreshMessages();
                                       }));
        current_socket->on("username", sio::socket::event_listener_aux(
                                           [&](string const &name, message::ptr const &data, bool isAck, message::list &ack_resp)
                                           { current_socket->emit("add", username); }));
        current_socket->on("users", sio::socket::event_listener_aux(
                                        [&](string const &name, message::ptr const &data, bool isAck, message::list &ack_resp)
                                        {
                                            json j = json::parse(data->get_string());
                                            users.clear();
                                            for (string user : j["users"])
                                                users.insert(user);
                                            refreshUsers();
                                        }));
    }

    Application(int height, int width, string username) : username(username), selectedWindow(CHAT), message(""), messageScroll(0)
    {
        initscr();

        raw();
        noecho();

        start_color();
        init_pair(0, COLOR_BLACK, COLOR_BLACK);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);

        BLACK_ON_BLACK = COLOR_PAIR(0);
        RED_ON_BLACK = COLOR_PAIR(1);
        GREEN_ON_BLACK = COLOR_PAIR(2);
        YELLOW_ON_BLACK = COLOR_PAIR(3);
        BLUE_ON_BLACK = COLOR_PAIR(4);
        MAGENTA_ON_BLACK = COLOR_PAIR(5);
        CYAN_ON_BLACK = COLOR_PAIR(6);
        WHITE_ON_BLACK = COLOR_PAIR(7);

        winUsersBorder = newwin(height, width / 4, 0, 0);
        winUsers = newwin(height - 2, (width / 4) - 2, 1, 1);
        winMessagesBorder = newwin(height - 3, (width - width / 4), 0, width / 4);
        winMessages = newwin(height - 6, (width - width / 4) - 2, 1, 1 + width / 4);
        winChatBorder = newwin(3, (width - width / 4), height - 3, width / 4);
        winChat = newwin(1, (width - width / 4) - 6, height - 2, width / 4 + 5);
        refresh();

        scrollok(winMessages, true);

        // users.insert("yash");
        // users.insert("shree");
        // users.insert("gaurav");

        // messages.push_back(Message("yash", "Hello!1"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));
        // messages.push_back(Message("yash", "Hello!2"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));
        // messages.push_back(Message("yash", "Hello!3"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));
        // messages.push_back(Message("yash", "Hello!4"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));
        // messages.push_back(Message("yash", "Hello!5"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));
        // messages.push_back(Message("yash", "Hello!6"));
        // messages.push_back(Message("shree", "Hello!"));
        // messages.push_back(Message("gaurav", "Hello!"));

        refreshWindows();

        io.connect(URL);
        current_socket = io.socket();

        bind_events();
    }
    void refreshUsersBorder()
    {
        if (selectedWindow == USERS)
            wattron(winUsersBorder, YELLOW_ON_BLACK);
        else
            wattron(winUsersBorder, WHITE_ON_BLACK);
        box(winUsersBorder, 0, 0);
        mvwprintw(winUsersBorder, 0, 0, "Users");
        wrefresh(winUsersBorder);
    }
    void refreshUsers()
    {
        wclear(winUsers);
        for (string user : users)
            wprintw(winUsers, string(user + "\n").c_str());
        wrefresh(winUsers);
    }
    void refreshMessagesBorder()
    {
        if (selectedWindow == MESSAGES)
            wattron(winMessagesBorder, YELLOW_ON_BLACK);
        else
            wattron(winMessagesBorder, WHITE_ON_BLACK);
        box(winMessagesBorder, 0, 0);
        mvwprintw(winMessagesBorder, 0, 0, "Messages");
        wrefresh(winMessagesBorder);
    }
    void refreshMessages()
    {
        wclear(winMessages);
        for (int i = 0; i < messages.size() - messageScroll; i++)
            wprintw(winMessages, string(messages[i].getUsername() + " : " + messages[i].getMessage() + "\n").c_str());
        wrefresh(winMessages);
    }
    void refreshChatBorder()
    {
        mvwprintw(winChatBorder, 1, 2, ">");
        if (selectedWindow == CHAT)
            wattron(winChatBorder, YELLOW_ON_BLACK);
        else
            wattron(winChatBorder, WHITE_ON_BLACK);
        box(winChatBorder, 0, 0);
        mvwprintw(winChatBorder, 0, 0, "Chat");
        wrefresh(winChatBorder);
    }
    void refreshChat()
    {
        wclear(winChat);
        mvwprintw(winChat, 0, 0, message.c_str());
        wrefresh(winChat);
    }
    void refreshWindows()
    {
        refreshUsersBorder();
        refreshMessagesBorder();
        refreshChatBorder();

        refreshUsers();
        refreshMessages();
        refreshChat();
    }

public:
    static Application *getInstance(int height, int width, string username)
    {
        if (instance == nullptr)
            instance = new Application(height, width, username);
        return instance;
    }

    void run()
    {
        int c = 0;
        while (c != 3)
        {
            c = getch();

            switch (c)
            {
            case 6:
                selectedWindow = USERS;
                refreshWindows();
                break;
            case 7:
                selectedWindow = MESSAGES;
                refreshWindows();
                break;
            case 8:
                selectedWindow = CHAT;
                refreshWindows();
                break;

            default:
                switch (selectedWindow)
                {
                case USERS:
                    break;
                case MESSAGES:
                    switch (c)
                    {
                    case 65:
                        if (((messages.size() - messageScroll) > winMessages->_maxy))
                        {
                            messageScroll++;
                            refreshMessages();
                        }
                        break;
                    case 66:
                        if (messageScroll != 0)
                        {
                            messageScroll--;
                            refreshMessages();
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                case CHAT:
                    if (c == 127 && message.size() > 0)
                        message.pop_back();
                    else if (c == 10)
                    {
                        if (message.size() > 0)
                            current_socket->emit("chat", Message(username, message).getJson().dump());
                        message = "";
                    }
                    else if (c != 127)
                        message += c;
                    refreshChat();
                    break;
                default:
                    break;
                }
            }
        }
        current_socket->emit("delete", username);
    }
    ~Application()
    {
        endwin();
        io.close();
    }
};
Application *Application::instance = nullptr;

int main(int argc, char **argv)
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    string username = "";
    if (argc == 2)
        username = argv[1];
    else
    {
        cout << "Enter username : ";
        cin >> username;
    }

    Application *app = Application::getInstance(size.ws_row, size.ws_col, username);
    app->run();
    delete app;
    return 0;
}