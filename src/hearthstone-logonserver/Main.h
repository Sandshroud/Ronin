/***
 * Demonstrike Core
 */

class AuthSocket;

extern bool mrunning;
extern Mutex _authSocketLock;
extern bool m_encryptedPasswords;
extern std::set<AuthSocket*> _authSockets;

struct AllowedIP
{
    unsigned int IP;
    unsigned char Bytes;
};

bool IsServerAllowed(unsigned int IP);
bool IsServerAllowedMod(unsigned int IP);

class LogonServer;
class LogonServer : public Singleton< LogonServer >
{
public:
    void CheckForDeadSockets();
    void Run(int argc, char ** argv);
    void Stop();
    uint32 max_build;
    uint32 min_build;
    uint8 sql_hash[20];

private:
    bool m_stopEvent;
};
