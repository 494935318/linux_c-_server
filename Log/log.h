#include "utils.h"
#include "buffer.h"
#include "locker.h"
#include <sys/time.h>
#define LOG_TRACE                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::TRACE) \
    \ 
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__)         \
        .stream()
#define LOG_DEBUG                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::DEBUG) \
    \ 
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__)         \
        .stream()
#define LOG_INFO                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::INFO) \
    \ 
    Logger(__FILE__, __LINE__)                                 \
        .stream()
#define LOG_WARN                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::WARN) \
    \
    Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::ERROR) \
    \
    Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL                                               \
    if (Logger::can_log && Logger::logLevel() <= Logger::FATAL) \
    \
    Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR                                              \
    if (Logger::can_log && Logger::logLevel() <= Logger::FATAL) \
\
    Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL                                            \
    if (Logger::can_log && Logger::logLevel() <= Logger::FATAL) \
    \
    Logger(__FILE__, __LINE__, true).stream()
typedef Buffer::Fixed_Buffer<4 * 1024 * 1024, 32> big_buff;
typedef shared_ptr<big_buff> Bufferptr;
typedef vector<Bufferptr> BufferVector;

class log_file : noncopyable
{
public:
    log_file();
    ~log_file();
    void write(char *, int len);
    FILE *f;
    long long f_size;
    static string exec_name;
    tm my_tm;
};

class log_stream : noncopyable
{
public:
    log_stream(){

    };
    log_stream &operator<<(const string &a)
    {
        buff.append(a);
        return *this;
    };
    log_stream &operator<<(double a)
    {
        buff.append(to_string(a));
        return *this;
    };
    log_stream &operator<<(int a)
    {
        buff.append(to_string(a));
        return *this;
    };
    log_stream &operator<<(size_t a)
    {
        buff.append(to_string(a));
        return *this;
    };
    log_stream &operator<<(const char &a)
    {
        buff.append(&a, 1);
        return *this;
    };
    log_stream &operator<<(unsigned int a)
    {
        buff.append(to_string(a));
        return *this;
    };
    log_stream &operator<<(const char *&a)
    {
        buff.append(a, strlen(a));
        return *this;
    };
    Buffer::Buffer buff;
};
class Async_loging;
class Logger : noncopyable
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    class SourceFile
    {
    public:
        template <int N>
        inline SourceFile(const char (&arr)[N])
            : m_data(arr),
              m_size(N - 1)
        {
            const char *slash = strrchr(m_data, '/'); // builtin function
            if (slash)
            {
                m_data = slash + 1;
                m_size -= static_cast<int>(m_data - arr);
            }
        }

        explicit SourceFile(const char *filename)
            : m_data(filename)
        {
            const char *slash = strrchr(filename, '/');
            if (slash)
            {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
        }

        const char *m_data;
        int m_size;
    };
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char *func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();
    log_stream &stream()
    {
        return m_impl.m_stream;
    };
    static void setLogLevel(LogLevel level);
    static LogLevel logLevel();
    static LogLevel g_logLevel;
    static void stop_log();
    static void start_log();
    static bool can_log;

private:
    static void init();

    class logger_impl : noncopyable
    {
    public:
        typedef Logger::LogLevel LogLevel;
        logger_impl(const SourceFile &file, LogLevel level, int old_errno, int line);
        void finish();
        void formatTime();
        LogLevel m_level;
        log_stream m_stream;
        int m_line;
        SourceFile m_file;
        timeval m_time;
    };

private:
    static shared_ptr<Async_loging> Async_log;
    logger_impl m_impl;
};
class Async_loging : noncopyable
{
public:
    Async_loging();
    void work();

    void append(const char *, int len);
    static void *thread_run(void *);
    void thread_run();
    static void *time_run(void *);

    void update_file();
    bool runing;
    shared_ptr<log_file> f;
};
void cond_time();