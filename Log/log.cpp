#include "log.h"
#include "event_loop.h"
#define MAX_EMPTY_BUFFER 10
static const long long LOG_FILESIZE = 1024 * 1024 * 10;
BufferVector fullBuffers, empty_buffers;
Bufferptr Current_buffer;
locker mutex_, empty_mutex;
cond write_cond;
__thread char t_time[64];
__thread time_t t_lastSecond;
__thread char t_errnobuf[512];
// static Async_loging Async_log;
const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "[TRACE]",
        "[DEBUG]",
        "[INFO ]",
        "[WARN ]",
        "[ERROR]",
        "[FATAL]",
};
string ReadLink()
{
    char name[100];
    int rval = readlink("/proc/self/exe", name, sizeof(name) - 1);
    if (rval == -1)
    {
        cout << "readlink error" << endl;
    }
    name[rval] = '\0';
    return string(strrchr(name, '/') + 1);
}
string log_file::exec_name = ReadLink();
Logger::Logger(SourceFile file, int line) : m_impl(file, INFO, 0, line)
{

    // init();
};
Logger::Logger(SourceFile file, int line, LogLevel level) : m_impl(file, level, 0, line)
{
    // init();
};
Logger::Logger(SourceFile file, int line, LogLevel level, const char *func) : m_impl(file, level, 0, line)
{
    // init();
    stream() << '[' << func << ']'<<' ';
};
Logger::Logger(SourceFile file, int line, bool toAbort) : m_impl(file, toAbort ? FATAL : ERROR, errno, line)
{
    // init();
};
Logger::~Logger()
{
    m_impl.finish();
    Async_log->append(m_impl.m_stream.buff.begin(), m_impl.m_stream.buff.readable_size());
};
 void Logger::setLogLevel(LogLevel level)
{
    g_logLevel = level;
};
 Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
};
Logger::LogLevel  Logger::g_logLevel = Logger::LogLevel::TRACE;
void Logger::stop_log()
{
    can_log = false;
};
// 打开记录
void Logger::start_log()
{
    init();
    can_log = true;
};
bool Logger::can_log = false;
Logger::logger_impl::logger_impl(const SourceFile &file, LogLevel level, int old_errno, int line)
    : m_file(file),
      m_line(line),
      m_level(level),
      m_stream()
{
    gettimeofday(&m_time, NULL);
    formatTime();
    m_stream << LogLevelName[m_level] << ' ';
    m_stream << '[' << m_file.m_data << ':' << m_line << ']'<<" ";
    if (old_errno)
    {
        m_stream << strerror(old_errno) << "(errno=" << old_errno << ')'<<" ";
    }
    m_stream<<"pid:"<<current_thread_id()<<" ";
};
void Logger::logger_impl::finish()
{
    m_stream << "\n";
};
void Logger::logger_impl::formatTime()
{
    time_t seconds = m_time.tv_sec;
    if (m_time.tv_sec != t_lastSecond)
    {
        t_lastSecond = m_time.tv_sec;
        struct tm tm_time;

        ::localtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime

        int len = sprintf(t_time, "%4d-%02d-%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 19);
    }
    char us[8];
    sprintf(us,".%06d", m_time.tv_usec);
    m_stream << t_time << us;
};
void Logger::init() {
        if (!Async_log)
        {
            Async_log.reset(new Async_loging());
            Async_log->work();
        }
    }
shared_ptr<Async_loging> Logger::Async_log=0;
log_file::log_file()
{   static int id=0;
    // time_t t = time(NULL);
    id=id%10000;
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    localtime_r(&t, &my_tm);
    char tail[256] = {'\0'};
    if(!isdir("./log")){
        mkdir("./log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    sprintf(tail, "./log/%s%dy%02dm%02dd%02dh%02dm%02ds_%d.log", exec_name.c_str(), my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec,id++);
    f = fopen(tail, "a");
    f_size = 0;
};
log_file::~log_file()
{
    fflush(f);
    fclose(f);
};
void log_file::write(char *a, int len)
{
    fwrite(a, sizeof(char), len, f);
    
    f_size += len;
    
};

Async_loging::Async_loging()
{
    timeval t;
    gettimeofday(&t, nullptr);
    t_lastSecond = t.tv_sec;
    t_lastSecond = t.tv_sec;
    struct tm tm_time;

    ::localtime_r(&t.tv_sec, &tm_time); // FIXME TimeZone::fromUtcTime

    int len = sprintf(t_time, "%4d-%02d-%02d %02d:%02d:%02d",
                       tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                       tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec);
};
void Async_loging::append(const char *a, int len)
{
    lock_guard lock(mutex_);
    if (Current_buffer&&Current_buffer->avail() >= len)
    {
        Current_buffer->append(a, len);
    }
    else
    {   if(Current_buffer)
        fullBuffers.push_back(Current_buffer);

        {
            lock_guard empty_guard(empty_mutex);
            if (empty_buffers.size() > 0)
            {
                Current_buffer = move(empty_buffers.back());
                empty_buffers.pop_back();
            }
            else
            {
                Current_buffer.reset(new big_buff);
            }
        }
        Current_buffer->append(a, len);
        write_cond.signal();
    }
};
void *Async_loging::thread_run(void *arg)
{
    auto a = static_cast<Async_loging *>(arg);
    a->thread_run();
    return 0;
}
void Async_loging::thread_run()
{
   
    // BufferVector buffersempty_tmp;
    while (true)
    {
         BufferVector bufferstowrite;
        {
            lock_guard lock(mutex_);
            if (fullBuffers.empty())
            {
                write_cond.wait(mutex_, bind(&BufferVector::size, &fullBuffers));
            }
            update_file();
            
            swap(bufferstowrite, fullBuffers);
        }

        for (int i = 0; i < bufferstowrite.size(); i++)
        {
            if (bufferstowrite[i]->readable_size() > 0)
            {
                f->write(bufferstowrite[i]->begin(), bufferstowrite[i]->readable_size());
                bufferstowrite[i]->retrieve_all();
            }
        }
        fflush(f->f);
        {
            lock_guard empty_(empty_mutex);
            if (empty_buffers.size() < MAX_EMPTY_BUFFER)
            {
                empty_buffers.insert(empty_buffers.end(), bufferstowrite.begin(), bufferstowrite.end());
            }
        }
    }
};
void *Async_loging::time_run(void *)
{
    event_loop loop;
    timeval t;
    t.tv_sec = 3;
    t.tv_usec = 0;
    loop.runevery(t, cond_time);
    loop.run();
    return 0;
};
void Async_loging::update_file()
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    tm my_tm;
    localtime_r(&t, &my_tm);
    if (!f || my_tm.tm_hour != f->my_tm.tm_hour || f->f_size >= LOG_FILESIZE)
    {
        f.reset(new log_file());
    }
};

void cond_time()
{
    lock_guard lock(mutex_);
    write_cond.signal();
    if(Current_buffer){
    fullBuffers.push_back(Current_buffer);
    Current_buffer.reset();
    }
}
void Async_loging::work()
{
    pthread_t *a1=new pthread_t(), *a2=new pthread_t();
    pthread_create(a1, NULL, Async_loging::time_run, NULL);
    pthread_create(a2, NULL, Async_loging::thread_run, this);
    pthread_detach(*a1);
    pthread_detach(*a2);
};
