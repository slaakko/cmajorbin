/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Debugger/Gdb.hpp>
#include <Cm.Debugger/GdbReply.hpp>
#include <Cm.Util/System.hpp>
#include <Cm.Util/TextUtils.hpp>
#include <iostream>

namespace Cm { namespace Debugger {

std::mutex pipeReadMutex;
std::mutex pipeWriteMutex;
std::mutex commandMutex;

ContinueReplyData::ContinueReplyData() : exitCode(0)
{
}

GdbCommand::GdbCommand(const std::string& message_) : message(message_)
{
}

GdbCommand::~GdbCommand()
{
}

void GdbCommand::SetReplyMessage(const std::string& replyMessage_)
{
    replyMessage = replyMessage_;
}

GdbSetWidthUnlimitedCommand::GdbSetWidthUnlimitedCommand() : GdbCommand("set width 0")
{
}

GdbSetHeightUnlimitedCommand::GdbSetHeightUnlimitedCommand() : GdbCommand("set height 0")
{
}

GdbStartCommand::GdbStartCommand() : GdbCommand("start")
{
}

GdbQuitCommand::GdbQuitCommand() : GdbCommand("quit")
{
}

GdbBreakCommand::GdbBreakCommand(const std::string& cFileLine_) : GdbCommand("break " + cFileLine_)
{
}

GdbContinueCommand::GdbContinueCommand() : GdbCommand("continue")
{
}

GdbClearCommand::GdbClearCommand(const std::string& cFileLine_) : GdbCommand("clear " + cFileLine_)
{
}

GdbBackTraceCommand::GdbBackTraceCommand() : GdbCommand("backtrace")
{
}

unsigned long long SpawnGdb(std::vector<std::string> args)
{
    std::string gdb = "gdb";
    unsigned long long gdbHandle = Cm::Util::Spawn(gdb, args);
    return gdbHandle;
}

void RunGdb(Gdb* gdb)
{
    gdb->DoRun();
}

Gdb::Gdb(const std::string& program_, const std::vector<std::string>& args_) : 
    program(program_), args(args_), gdbHandle(-1), hasReply(false), hasCommand(false), firstStart(true), gdbKilled(false)
{
    std::vector<std::string> gdbArgs;
    if (!args.empty())
    {
        gdbArgs.push_back("--args");
    }
    gdbArgs.push_back(program);
    if (!args.empty())
    {
        gdbArgs.insert(gdbArgs.end(), args.begin(), args.end());
    }
    std::vector<int> oldHandles;
    Cm::Util::RedirectStdHandlesToPipes(oldHandles, pipeHandles);
    gdbHandle = SpawnGdb(gdbArgs);
    Cm::Util::RestoreStdHandles(oldHandles);
}

void Gdb::Run()
{
    gdbThread = std::thread(RunGdb, this);
}

void Gdb::DoRun()
{
    try
    {
        std::string welcome = Read();
        while (true)
        {
            std::unique_lock<std::mutex> commandLock(commandReadyMtx);
            commandReady.wait(commandLock, [this] { return hasCommand; });
            hasCommand = false;
            std::shared_ptr<GdbCommand> cmd = commandToExecute;
            commandToExecute = nullptr;
            std::lock_guard<std::mutex> replyLock(replyReadyMtx);
            std::string message = cmd->Message();
            message.append("\n");
            Write(message);
            if (cmd->ReplyExpected())
            {
                if (cmd->IsContinueCommand())
                {
                    std::string replyMessage = ReadContinueReply();
                    cmd->SetReplyMessage(replyMessage);
                }
                else
                {
                    std::string replyMessage = Read();
                    cmd->SetReplyMessage(replyMessage);
                }
            }
            hasReply = true;
            replyReady.notify_one();
            if (cmd->IsQuitCommand()) break;
        }
    }
    catch (const std::exception& ex)
    {
        int x = 0;
    }
}

std::shared_ptr<GdbCommand> Gdb::Start()
{
    if (firstStart)
    {
        firstStart = false;
        ExecuteCommand(std::shared_ptr<GdbCommand>(new GdbSetWidthUnlimitedCommand()));
        ExecuteCommand(std::shared_ptr<GdbCommand>(new GdbSetHeightUnlimitedCommand()));
    }
    std::shared_ptr<GdbCommand> startCommand(new GdbStartCommand());
    ExecuteCommand(startCommand);
    return startCommand;
}

void Gdb::Quit()
{
    if (gdbHandle != -1)
    {
        ExecuteCommand(std::shared_ptr<GdbCommand>(new GdbQuitCommand()));
        Cm::Util::Wait(gdbHandle);
        gdbHandle = -1;
    }
    gdbThread.join();
}

std::shared_ptr<GdbCommand> Gdb::Break(const std::string& cFileLine)
{
    std::shared_ptr<GdbCommand> breakCommand(new GdbBreakCommand(cFileLine));
    ExecuteCommand(breakCommand);
    return breakCommand;
}

std::shared_ptr<GdbCommand> Gdb::Continue()
{
    std::shared_ptr<GdbCommand> continueCommand(new GdbContinueCommand());
    ExecuteCommand(continueCommand);
    return continueCommand;
}

std::shared_ptr<GdbCommand> Gdb::Clear(const std::string& cFileLine)
{
    std::shared_ptr<GdbCommand> clearCommand(new GdbClearCommand(cFileLine));
    ExecuteCommand(clearCommand);
    return clearCommand;
}

std::shared_ptr<GdbCommand> Gdb::BackTrace()
{
    std::shared_ptr<GdbCommand> backTraceCommand(new GdbBackTraceCommand());
    ExecuteCommand(backTraceCommand);
    return backTraceCommand;
}

std::string Gdb::Read()
{
    static char buf[8192];
    std::string gdbPrompt("(gdb) ");
    std::string message;
    while (!Cm::Util::EndsWith(message, gdbPrompt))
    {
        int bytesRead = -1;
        {
            std::lock_guard<std::mutex> lck(pipeReadMutex);
            bytesRead = Cm::Util::ReadFromPipe(pipeHandles[1], buf, sizeof(buf) - 1);
        }
        if (bytesRead == -1)
        {
            std::string errorMessage = strerror(errno);
            throw std::runtime_error(errorMessage);
        }
        if (bytesRead > 0)
        {
            message.append(Cm::Util::NarrowString(buf, bytesRead));
        }
        else
        {
            break;
        }
    }
    return Cm::Util::Trim(message.substr(0, message.length() - gdbPrompt.length()));
}

ContinueReplyGrammar* continueReplyGrammar = nullptr;

std::string Gdb::ReadContinueReply()
{
    if (!continueReplyGrammar)
    {
        continueReplyGrammar = ContinueReplyGrammar::Create();
    }
    static char buf[8192];
    std::string gdbPrompt("(gdb) ");
    ContinueReplyData data;
    ContinueReplyState state = ContinueReplyState::start;
    std::string combinedMessage;
    while (state != ContinueReplyState::prompt)
    {
        std::string message;
        int bytesRead = -1;
        {
            std::lock_guard<std::mutex> lck(pipeReadMutex);
            bytesRead = Cm::Util::ReadFromPipe(pipeHandles[1], buf, sizeof(buf) - 1);
        }
        if (bytesRead == -1)
        {
            std::string errorMessage = strerror(errno);
            throw std::runtime_error(errorMessage);
        }
        if (bytesRead > 0)
        {
            message.append(Cm::Util::NarrowString(buf, bytesRead));
            combinedMessage.append(message);
            std::string consoleLines;
            bool consoleLinesSet = false;
            std::vector<std::string> lines = Cm::Util::Split(message, '\n');
            for (const std::string& line : lines)
            {
                switch (state)
                {
                    case ContinueReplyState::start:
                    {
                        state = continueReplyGrammar->Parse(line.c_str(), line.c_str() + line.length(), 0, "", &data);
                        if (state == ContinueReplyState::consoleLine)
                        {
                            if (consoleLinesSet)
                            {
                                consoleLines.append("\n");
                            }
                            consoleLines.append(data.ConsoleLine());
                            consoleLinesSet = true;
                            state = ContinueReplyState::start;
                        }
                        else if (state == ContinueReplyState::exit)
                        {
                            state = ContinueReplyState::start;
                        }
                        else if (state == ContinueReplyState::continuing)
                        {
                            state = ContinueReplyState::start;
                        }
                        break;
                    }
                    case ContinueReplyState::breakpoint:
                    {
                        state = ContinueReplyState::start;
                        break;
                    }
                    case ContinueReplyState::signal:
                    {
                        state = ContinueReplyState::start;
                        break;
                    }
                }
            }
            if (consoleLinesSet)
            {
                std::cout << consoleLines;
                std::cout.flush();
            }
        }
    }
    return Cm::Util::Trim(combinedMessage.substr(0, combinedMessage.length() - gdbPrompt.length()));
}

void Gdb::Write(const std::string& message)
{
    int bytesWritten = -1;
    {
        std::lock_guard<std::mutex> lck(pipeWriteMutex);
        bytesWritten = Cm::Util::WriteToPipe(pipeHandles[0], (void*)message.c_str(), (unsigned int)message.length());
    }
    if (bytesWritten == -1)
    {
        std::string errorMessage = strerror(errno);
        throw std::runtime_error(errorMessage);
    }
}

void Gdb::ExecuteCommand(std::shared_ptr<GdbCommand>& command)
{
    hasReply = false;
    hasCommand = false;
    std::lock_guard<std::mutex> lock(commandMutex);
    {
        std::lock_guard<std::mutex> lck(commandReadyMtx);
        commandToExecute = command;
        hasCommand = true;
        commandReady.notify_one();
    }
    std::unique_lock<std::mutex> lck(replyReadyMtx);
    replyReady.wait(lck, [this] { return hasReply; });
    hasReply = false;
}

} } // Cm::Debugger
