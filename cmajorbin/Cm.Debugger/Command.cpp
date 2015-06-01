/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Debugger/Command.hpp>
#include <Cm.Debugger/Gdb.hpp>
#include <Cm.Debugger/InputReader.hpp>
#include <Cm.Debugger/DebugInfo.hpp>
#include <Cm.Debugger/Shell.hpp>
#include <Cm.Debugger/Util.hpp>
#include <iostream>

namespace Cm { namespace Debugger {

Command::~Command()
{
}

void Command::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
}

void StartCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    if (debugInfo.GetState() != State::idle)
    {
        throw std::runtime_error("already started");
    }
    std::shared_ptr<GdbCommand> startCommand = gdb.Start();
    debugInfo.SetState(State::debugging);
    Cm::Core::CFunctionDebugInfo* main = debugInfo.MainFunctionDebugInfo();
    if (!main)
    {
        throw std::runtime_error("main function debug info not set");
    }
    Cm::Core::CfgNode* mainEntry = main->Entry();
    debugInfo.SetCurrentNode(mainEntry);
    std::string mainEntryCFileLine = mainEntry->GetCFileLine().ToString();
    bool hasBreakpointAtMainEntryPoint = debugInfo.HasCBreakpoint(mainEntryCFileLine);
    if (!hasBreakpointAtMainEntryPoint)
    {
        std::shared_ptr<GdbCommand> breakCommand = gdb.Break(mainEntryCFileLine);
    }
    std::shared_ptr<GdbCommand> continueCommand = gdb.Continue();
    if (!hasBreakpointAtMainEntryPoint)
    {
        std::shared_ptr<GdbCommand> clearCommand = gdb.Clear(mainEntryCFileLine);
    }
    SourceFile* currentSourceFile = debugInfo.GetSourceFile(main->SourceFilePath());
    debugInfo.SetCurrentSourceFile(currentSourceFile);
    currentSourceFile->SetActiveLineNumber(mainEntry->GetSourceSpan().Line());
    currentSourceFile->SetListLineNumber(0);
    std::cout << "debugging started" << std::endl;
}

void QuitCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    gdb.Quit();
    inputReader.Exit();
    debugInfo.SetState(State::exit);
}

void HelpCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    inputReader.Proceed();
    std::cout <<
        "start:                         start debugging session\n"
        "q(uit):                        end debugging session and exit\n"
        "h(elp):                        print this help message\n"
        "c(ontinue):                    continue execution\n"
        "n(ext):                        goto next line\n"
        "s(tep):                        step into function\n"
        "o(ut):                         goto out of function\n"
        "b(reak) [SOURCEFILE:]LINE      set breakpoint at source file SOURCEFILE line LINE\n"
        "cl(ear) N                      clear breakpoint number N\n"
        "ca(llstack)                    print callstack\n"
        "f(rame) N                      set frame number N (0 is current)\n"
        "l(ist) [SOURCEFILE:]LINE       list source code around line LINE\n"
        "l(ist)*                        list source code around current position\n"
        "l(ist)                         list next lines\n"
        "i(nspect) EXPRESSION           inspect value of EXPRESSION\n"
        "show breakpoints               show breakpoints\n"
        "show libraries                 show debuggable libraries\n"
        "set debug library <lib> on     enable debugging library <lib>\n"
        "set debug library <lib> off    disable debugging library <lib>\n"
        "set break on throw on          enable break when exception is thrown\n"
        "set break on throw off         disable break when exception is thrown\n"
        "<empty line>                   repeat last command\n"
        << "\n";
}

struct ExecContinue
{
    ExecContinue(Gdb& gdb_, InputReader& inputReader_) : gdb(gdb_), inputReader(inputReader_)
    {
        inputReader.StartRedirecting();
    }
    std::shared_ptr<GdbCommand> Continue()
    {
        return gdb.Continue();
    }
    ~ExecContinue()
    {
        inputReader.StopRedirecting();
    }
    Gdb& gdb;
    InputReader& inputReader;
};

void ContinueCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    if (debugInfo.GetState() == State::idle)
    {
        throw std::runtime_error("not started");
    }
    std::cout << "continuing" << std::endl;
    ExecContinue exec(gdb, inputReader);
    std::shared_ptr<GdbCommand> continueCommand = exec.Continue();
    std::shared_ptr<GdbCommand> backTraceCommand = gdb.BackTrace();
    CallStack callStack = shell.ParseBackTraceReply(backTraceCommand->ReplyMessage());
    if (callStack.Frames().empty())
    {
        debugInfo.SetCurrentNode(nullptr);
        debugInfo.SetState(State::idle);
        std::cout << "program exited" << std::endl;
    }
    else
    {
        Breakpoint* bp = debugInfo.GetBreakpoint(callStack.Frames()[0].SourceFileLine().ToString());
        if (bp)
        {
            Cm::Core::CfgNode* currentNode = bp->Node();
            debugInfo.SetCurrentNode(currentNode);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(currentNode->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), currentNode->GetSourceSpan().Line());
            std::cout << "breakpoint " << bp->Number() << " at " << sourceFileLine.ToString() << std::endl;
            currentSourceFile->SetActiveLineNumber(currentNode->GetSourceSpan().Line());
            currentSourceFile->ListLine(currentNode->GetSourceSpan().Line(), debugInfo.Breakpoints());
        }
        else
        {
            throw std::runtime_error("breakpoint not found");
        }
    }
}

NextCommand::NextCommand() : fromOut(false)
{

}

void NextCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    if (debugInfo.GetState() == State::idle)
    {
        throw std::runtime_error("not started");
    }
    SourceFile* currentSourceFile = debugInfo.GetCurrentSourceFile();
    if (currentSourceFile)
    {
        currentSourceFile->ResetListLineNumber();
    }
    std::shared_ptr<GdbCommand> backTraceBeforeCommand = gdb.BackTrace();
    CallStack callStackBefore = shell.ParseBackTraceReply(backTraceBeforeCommand->ReplyMessage());
    if (callStackBefore.Frames().empty())
    {
        throw std::runtime_error("program not running");
    }
    const Cm::Core::SourceFileLine& cFileLineBefore = callStackBefore.Frames()[0].SourceFileLine();
    Cm::Core::CfgNode* nodeBefore = debugInfo.GetNodeByCFileLine(cFileLineBefore, false);
    if (nodeBefore)
    {
        debugInfo.SetCurrentNode(nodeBefore);
    }
    else
    {
        Cm::Core::CFunCall* funCallBefore = debugInfo.GetFunCallByCFileLine(cFileLineBefore);
        if (funCallBefore)
        {
            Cm::Core::CfgNode* node = funCallBefore->Node();
            if (!node)
            {
                throw std::runtime_error("function call node not found");
            }
            debugInfo.SetCurrentNode(node);
        }
    }
    while (true)
    {
        Cm::Core::CfgNode* currentNode = debugInfo.CurrentNode();
        std::vector<std::string> nextCFileLines;
        if (!currentNode->Function()->IsMain() && callStackBefore.Frames().size() > 1)
        {
            const Cm::Core::SourceFileLine& parentCallLine = callStackBefore.Frames()[1].SourceFileLine();
            std::string returnCFileLine = Cm::Core::SourceFileLine(parentCallLine.SourceFilePath(), parentCallLine.SourceLineNumber() + 1).ToString();
            std::shared_ptr<GdbCommand> breakCommand = gdb.Break(returnCFileLine);
            nextCFileLines.push_back(returnCFileLine);
        }
        for (int nextNodeId : currentNode->Next())
        {
            Cm::Core::CfgNode* nextNode = currentNode->Function()->Cfg().GetNode(nextNodeId);
            std::string nextCFileLine = nextNode->GetCFileLine().ToString();
            bool hasBreakpoint = debugInfo.HasCBreakpoint(nextCFileLine);
            if (!hasBreakpoint)
            {
                std::shared_ptr<GdbCommand> breakCommand = gdb.Break(nextCFileLine);
                nextCFileLines.push_back(nextCFileLine);
            }
        }
        ExecContinue exec(gdb, inputReader);
        std::shared_ptr<GdbCommand> continueCommand = exec.Continue();
        for (const std::string& nextCFileLine : nextCFileLines)
        {
            std::shared_ptr<GdbCommand> clearCommand = gdb.Clear(nextCFileLine);
        }
        std::shared_ptr<GdbCommand> backTraceAfterCommand = gdb.BackTrace();
        CallStack callStackAfter = shell.ParseBackTraceReply(backTraceAfterCommand->ReplyMessage());
        if (callStackAfter.Frames().empty())
        {
            debugInfo.SetCurrentNode(nullptr);
            debugInfo.SetState(State::idle);
            std::cout << "program exited" << std::endl;
            return;
        }
        const Cm::Core::SourceFileLine& cFileLineAfter = callStackAfter.Frames()[0].SourceFileLine();
        Breakpoint* bp = debugInfo.GetBreakpoint(cFileLineAfter.ToString());
        if (bp)
        {
            Cm::Core::CfgNode* nodeAfter = bp->Node();
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
            std::cout << "breakpoint " << bp->Number() << " at " << sourceFileLine.ToString() << std::endl;
            currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
            currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
            return;
        }
        Cm::Core::CfgNode* nodeAfter = debugInfo.GetNodeByCFileLine(cFileLineAfter, false);
        if (nodeAfter)
        {
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            if (callStackAfter.Frames().size() <= callStackBefore.Frames().size() && !nodeAfter->GetSourceSpan().IsNull())
            {
                if (fromOut || nodeBefore && nodeBefore->Function() != nodeAfter->Function())
                {
                    Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
                    std::cout << "at " << sourceFileLine.ToString() << std::endl;
                }
                currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
                currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
                return;
            }
        }
        Cm::Core::CFunCall* funCallAfter = debugInfo.GetFunCallByCFileLine(cFileLineAfter, false);
        if (funCallAfter)
        {
            Cm::Core::CfgNode* nodeAfter = funCallAfter->Node();
            if (!nodeAfter)
            {
                throw std::runtime_error("function call node not set");
            }
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
        }
    }
}

void StepCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    if (debugInfo.GetState() == State::idle)
    {
        throw std::runtime_error("not started");
    }
    SourceFile* currentSourceFile = debugInfo.GetCurrentSourceFile();
    if (currentSourceFile)
    {
        currentSourceFile->ResetListLineNumber();
    }
    std::shared_ptr<GdbCommand> backTraceBeforeCommand = gdb.BackTrace();
    CallStack callStackBefore = shell.ParseBackTraceReply(backTraceBeforeCommand->ReplyMessage());
    const Cm::Core::SourceFileLine& cFileLineBefore = callStackBefore.Frames()[0].SourceFileLine();
    int funCallIndex = 0;
    Cm::Core::CfgNode* nodeBefore = debugInfo.GetNodeByCFileLine(cFileLineBefore, false);
    if (nodeBefore)
    {
        debugInfo.SetCurrentNode(nodeBefore);
    }
    else
    {
        Cm::Core::CFunCall* funCallBefore = debugInfo.GetFunCallByCFileLine(cFileLineBefore);
        if (funCallBefore)
        {
            funCallIndex = funCallBefore->Index() + 1;
            Cm::Core::CfgNode* node = funCallBefore->Node();
            if (!node)
            {
                throw std::runtime_error("function call node not found");
            }
            debugInfo.SetCurrentNode(node);
        }
    }
    while (true)
    {
        Cm::Core::CfgNode* currentNode = debugInfo.CurrentNode();
        std::vector<std::string> nextCFileLines;
        if (!currentNode->Function()->IsMain() && callStackBefore.Frames().size() > 1)
        {
            const Cm::Core::SourceFileLine& parentCallLine = callStackBefore.Frames()[1].SourceFileLine();
            std::string returnCFileLine = Cm::Core::SourceFileLine(parentCallLine.SourceFilePath(), parentCallLine.SourceLineNumber() + 1).ToString();;
            std::shared_ptr<GdbCommand> breakCommand = gdb.Break(returnCFileLine);
            nextCFileLines.push_back(returnCFileLine);
        }
        std::unordered_set<Cm::Core::CfgNode*> entryNodes;
        Cm::Core::CFunCall* cFunCall = currentNode->GetCFunCall(funCallIndex);
        if (cFunCall)
        {
            for (const std::string& funName : cFunCall->FunNames())
            {
                Cm::Core::CFunctionDebugInfo* funDebugInfo = debugInfo.GetFunctionDebugInfo(funName);
                if (funDebugInfo)
                {
                    Cm::Core::CfgNode* entryNode = funDebugInfo->Entry();
                    entryNodes.insert(entryNode);
                    std::string entryCFileLine = entryNode->GetCFileLine().ToString();
                    bool hasBreakpoint = debugInfo.HasCBreakpoint(entryCFileLine);
                    if (!hasBreakpoint)
                    {
                        std::shared_ptr<GdbCommand> breakCommand = gdb.Break(entryCFileLine);
                        nextCFileLines.push_back(entryCFileLine);
                    }
                    std::string callReturnLine = cFunCall->GetReturnCFileLine().ToString();
                    std::shared_ptr<GdbCommand> breakCommand = gdb.Break(callReturnLine);
                    nextCFileLines.push_back(callReturnLine);
                }
            }
        }
        for (int nextNodeId : currentNode->Next())
        {
            Cm::Core::CfgNode* nextNode = currentNode->Function()->Cfg().GetNode(nextNodeId);
            std::string nextCFileLine = nextNode->GetCFileLine().ToString();
            bool hasBreakpoint = debugInfo.HasCBreakpoint(nextCFileLine);
            if (!hasBreakpoint)
            {
                std::shared_ptr<GdbCommand> breakCommand = gdb.Break(nextCFileLine);
                nextCFileLines.push_back(nextCFileLine);
            }
        }
        ExecContinue exec(gdb, inputReader);
        std::shared_ptr<GdbCommand> continueCommand = exec.Continue();
        for (const std::string& nextCFileLine : nextCFileLines)
        {
            std::shared_ptr<GdbCommand> clearCommand = gdb.Clear(nextCFileLine);
        }
        std::shared_ptr<GdbCommand> backTraceAfterCommand = gdb.BackTrace();
        CallStack callStackAfter = shell.ParseBackTraceReply(backTraceAfterCommand->ReplyMessage());
        if (callStackAfter.Frames().empty())
        {
            debugInfo.SetCurrentNode(nullptr);
            debugInfo.SetState(State::idle);
            std::cout << "program exited" << std::endl;
            return;
        }
        const Cm::Core::SourceFileLine& cFileLineAfter = callStackAfter.Frames()[0].SourceFileLine();
        Breakpoint* bp = debugInfo.GetBreakpoint(cFileLineAfter.ToString());
        if (bp)
        {
            Cm::Core::CfgNode* nodeAfter = bp->Node();
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
            std::cout << "breakpoint " << bp->Number() << " at " << sourceFileLine.ToString() << std::endl;
            currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
            currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
            return;
        }
        Cm::Core::CfgNode* nodeAfter = debugInfo.GetNodeByCFileLine(cFileLineAfter, false);
        if (nodeAfter)
        {
            funCallIndex = 0;
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            bool nodeAfterIsEntryNode = entryNodes.find(nodeAfter) != entryNodes.end();
            if (nodeAfterIsEntryNode || callStackAfter.Frames().size() <= callStackBefore.Frames().size() && !nodeAfter->GetSourceSpan().IsNull())
            {
                if (nodeAfterIsEntryNode || nodeBefore && nodeBefore->Function() != nodeAfter->Function())
                {
                    Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
                    std::cout << "at " << sourceFileLine.ToString() << std::endl;
                }
                currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
                currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
                return;
            }
        }
        Cm::Core::CFunCall* funCallAfter = debugInfo.GetFunCallByCFileLine(cFileLineAfter, false);
        if (funCallAfter)
        {
            Cm::Core::CfgNode* nodeAfter = funCallAfter->Node();
            if (!nodeAfter)
            {
                throw std::runtime_error("function call node not set");
            }
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            ++funCallIndex;
        }
    }
}

void OutCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    if (debugInfo.GetState() == State::idle)
    {
        throw std::runtime_error("not started");
    }
    SourceFile* currentSourceFile = debugInfo.GetCurrentSourceFile();
    if (currentSourceFile)
    {
        currentSourceFile->ResetListLineNumber();
    }
    std::shared_ptr<GdbCommand> backTraceBeforeCommand = gdb.BackTrace();
    CallStack callStackBefore = shell.ParseBackTraceReply(backTraceBeforeCommand->ReplyMessage());
    if (callStackBefore.Frames().empty())
    {
        throw std::runtime_error("program not running");
    }
    const Cm::Core::SourceFileLine& cFileLineBefore = callStackBefore.Frames()[0].SourceFileLine();
    Cm::Core::CfgNode* nodeBefore = debugInfo.GetNodeByCFileLine(cFileLineBefore, false);
    if (nodeBefore)
    {
        debugInfo.SetCurrentNode(nodeBefore);
    }
    else
    {
        Cm::Core::CFunCall* funCallBefore = debugInfo.GetFunCallByCFileLine(cFileLineBefore);
        if (funCallBefore)
        {
            Cm::Core::CfgNode* node = funCallBefore->Node();
            if (!node)
            {
                throw std::runtime_error("function call node not found");
            }
            debugInfo.SetCurrentNode(node);
        }
    }
    while (true)
    {
        Cm::Core::CfgNode* currentNode = debugInfo.CurrentNode();
        std::vector<std::string> nextCFileLines;
        if (!currentNode->Function()->IsMain() && callStackBefore.Frames().size() > 1)
        {
            const Cm::Core::SourceFileLine& parentCallLine = callStackBefore.Frames()[1].SourceFileLine();
            std::string returnCFileLine = Cm::Core::SourceFileLine(parentCallLine.SourceFilePath(), parentCallLine.SourceLineNumber() + 1).ToString();
            std::shared_ptr<GdbCommand> breakCommand = gdb.Break(returnCFileLine);
            nextCFileLines.push_back(returnCFileLine);
        }
        ExecContinue exec(gdb, inputReader);
        std::shared_ptr<GdbCommand> continueCommand = exec.Continue();
        for (const std::string& nextCFileLine : nextCFileLines)
        {
            std::shared_ptr<GdbCommand> clearCommand = gdb.Clear(nextCFileLine);
        }
        std::shared_ptr<GdbCommand> backTraceAfterCommand = gdb.BackTrace();
        CallStack callStackAfter = shell.ParseBackTraceReply(backTraceAfterCommand->ReplyMessage());
        if (callStackAfter.Frames().empty())
        {
            debugInfo.SetCurrentNode(nullptr);
            debugInfo.SetState(State::idle);
            std::cout << "program exited" << std::endl;
            return;
        }
        const Cm::Core::SourceFileLine& cFileLineAfter = callStackAfter.Frames()[0].SourceFileLine();
        Breakpoint* bp = debugInfo.GetBreakpoint(cFileLineAfter.ToString());
        if (bp)
        {
            Cm::Core::CfgNode* nodeAfter = bp->Node();
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
            std::cout << "breakpoint " << bp->Number() << " at " << sourceFileLine.ToString() << std::endl;
            currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
            currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
            return;
        }
        Cm::Core::CfgNode* nodeAfter = debugInfo.GetNodeByCFileLine(cFileLineAfter, false);
        if (nodeAfter)
        {
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            if (callStackAfter.Frames().size() <= callStackBefore.Frames().size() && !nodeAfter->GetSourceSpan().IsNull())
            {
                Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
                std::cout << "at " << sourceFileLine.ToString() << std::endl;
                currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
                currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
                return;
            }
        }
        Cm::Core::CFunCall* funCallAfter = debugInfo.GetFunCallByCFileLine(cFileLineAfter, false);
        if (funCallAfter)
        {
            if (funCallAfter->IsLastCall())
            {
                NextCommand nextCommand;
                nextCommand.SetFromOut();
                nextCommand.Execute(debugInfo, gdb, inputReader, shell);
                return;
            }
            Cm::Core::CfgNode* nodeAfter = funCallAfter->Node();
            if (!nodeAfter)
            {
                throw std::runtime_error("function call node not set");
            }
            debugInfo.SetCurrentNode(nodeAfter);
            SourceFile* currentSourceFile = debugInfo.GetSourceFile(nodeAfter->Function()->SourceFilePath());
            debugInfo.SetCurrentSourceFile(currentSourceFile);
            if (callStackAfter.Frames().size() <= callStackBefore.Frames().size() && !nodeAfter->GetSourceSpan().IsNull())
            {
                Cm::Core::SourceFileLine sourceFileLine(currentSourceFile->FilePath(), nodeAfter->GetSourceSpan().Line());
                std::cout << "at " << sourceFileLine.ToString() << std::endl;
                currentSourceFile->SetActiveLineNumber(nodeAfter->GetSourceSpan().Line());
                currentSourceFile->ListLine(nodeAfter->GetSourceSpan().Line(), debugInfo.Breakpoints());
                return;
            }
        }
    }
}

BreakCommand::BreakCommand(const Cm::Core::SourceFileLine& sourceFileLine_) : sourceFileLine(sourceFileLine_)
{
}

void BreakCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    Cm::Core::CfgNode* node = debugInfo.GetNode(sourceFileLine);
    if (node)
    {
        Cm::Core::SourceFileLine cFileLine = node->GetCFileLine();
        std::shared_ptr<GdbCommand> breakCommand = gdb.Break(cFileLine.ToString());
        int bpNum = debugInfo.SetBreakpoint(new Breakpoint(cFileLine.ToString(), node));
        Cm::Core::SourceFileLine bpLine(node->Function()->SourceFilePath(), node->GetSourceSpan().Line());
        std::cout << "breakpoint " << bpNum << " set at " << bpLine.ToString() << std::endl;
    }
    else
    {
        throw std::runtime_error("node for '" + sourceFileLine.ToString() + "' not found");
    }
}

ClearCommand::ClearCommand(int bpNum_) : bpNum(bpNum_)
{
}

void ClearCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    Breakpoint* bp = debugInfo.GetBreakpoint(bpNum);
    if (bp)
    {
        std::shared_ptr<GdbCommand> clearCommand = gdb.Clear(bp->CFileLine());
        debugInfo.RemoveBreakpoint(bpNum);
        std::cout << "breakpoint " << bpNum << " deleted" << std::endl;
    }
    else
    {
        throw std::runtime_error("breakpoint number " + std::to_string(bpNum) + " not found");
    }
}

ListCommand::ListCommand(const Cm::Core::SourceFileLine& sourceFileLine_) : sourceFileLine(sourceFileLine_)
{
}

ListCommand::ListCommand(const std::string& command_) : command(command_)
{
}

void ListCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    SourceFile* sourceFile = debugInfo.GetCurrentSourceFile();
    if (!sourceFileLine.SourceFilePath().empty())
    {
        sourceFile = debugInfo.GetSourceFile(sourceFileLine.SourceFilePath());
        if (sourceFile)
        {
            debugInfo.SetCurrentSourceFile(sourceFile);
        }
    }
    if (!sourceFile)
    {
        throw std::runtime_error("current source file not set");
    }
    if (!sourceFileLine.IsEmpty())
    {
        sourceFile->SetListLineNumber(sourceFileLine.SourceLineNumber());
    }
    if (command == "*")
    {
        sourceFile->ResetListLineNumber();
    }
    sourceFile->List(debugInfo.Breakpoints());
    shell.SetLastCommand(CommandPtr(new ListCommand("+")));
}

void CallStackCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    std::shared_ptr<GdbCommand> backTraceCommand = gdb.BackTrace();
    CallStack callStack = shell.ParseBackTraceReply(backTraceCommand->ReplyMessage());
    int n = int(callStack.Frames().size());
    for (int i = 0; i < n; ++i)
    {
        const Frame& frame = callStack.Frames()[i];
        const Cm::Core::SourceFileLine& cFileLine = frame.SourceFileLine();
        Cm::Core::CfgNode* node = debugInfo.GetNodeByCFileLine(cFileLine, false);
        if (node)
        {
            std::cout << "#" << i << " " << node->Function()->FunctionDisplayName() << " at " << node->Function()->SourceFilePath() << ":" << node->GetSourceSpan().Line() << std::endl;
        }
        else
        {
            Cm::Core::SourceFileLine returnCFileLine(frame.SourceFileLine().SourceFilePath(), frame.SourceFileLine().SourceLineNumber() + 1);
            Cm::Core::CFunCall* call = debugInfo.GetFunCallByCFileLine(returnCFileLine, false);
            if (call)
            {
                Cm::Core::CfgNode* node = call->Node();
                std::cout << "#" << i << " " << node->Function()->FunctionDisplayName() << " at " << node->Function()->SourceFilePath() << ":" << node->GetSourceSpan().Line() << std::endl;
            }
        }
    }
}

FrameCommand::FrameCommand(int frameNumber_) : frameNumber(frameNumber_)
{
}

void FrameCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    // todo
}

void ShowBreakpointsCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    for (const std::pair<int, const std::unique_ptr<Breakpoint>&>& p : debugInfo.Breakpoints())
    {
        const std::unique_ptr<Breakpoint>& bp = p.second;
        Cm::Core::CfgNode* node = bp->Node();
        Cm::Core::SourceFileLine sourceFileLine(node->Function()->SourceFilePath(), node->GetSourceSpan().Line());
        std::cout << "breakpoint " << bp->Number() << " at " << sourceFileLine.ToString() << std::endl;
    }
}

void ShowLibrariesCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    // todo
}

SetDebugLibraryStatusCommand::SetDebugLibraryStatusCommand(const std::string& libName_, bool enabled_) : libName(libName_), enabled(enabled_)
{
}

void SetDebugLibraryStatusCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    // todo
}

SetBreakOnThrowCommand::SetBreakOnThrowCommand(bool enabled_) : enabled(enabled_)
{
}

void SetBreakOnThrowCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    // todo
}

InspectCommand::InspectCommand(const std::string& expr_) : expr(expr_)
{
}

void InspectCommand::Execute(DebugInfo& debugInfo, Gdb& gdb, InputReader& inputReader, Shell& shell)
{
    // todo
}

} } // Cm::Debugger
