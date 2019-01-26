
/// Chapter 2 - Communicating Sequential Processes
///
/// This chapter explores the interaction of the user interface, the state of
/// the application, and the rendering of the state of the application. This
/// structure corresponds to the so-called Model-View-Controller (MVC) application
/// architecture, but this chapter is going to avoid jargon and stick to calling
/// things more directly what they are. For reference, a user interface is a
/// controller, the renderer is a view, and the application state is the model.
/// The explanation in this chapter gains nothing from jargon, so it sticks to
/// UI, renderer, and application context.
///
/// Chapter 2 builds upon Chapter 1.

#define LABTEXT_ODR
#include "LabText.h"
#undef LABTEXT_ODR
#include "chapter1.cpp"

/// The build of chapter2 will be similar to that of chapter 1.
///
/// ~~~~
/// stddoc < chapter2.cpp > chapter2.html
/// mkdir gusteau-build-chapter2
/// cd gusteau-build-chapter2
/// cmake ../gusteau -DCHAPTER=chapter2 -G "generator"
/// ~~~~
/// where "generator" might be "Visual Studio 15 2017 Win64" for visual studio,
/// "Xcode" for xcode, or "Make" for make.
///
/// When cmake completes build and run the resulting project.
///
/// A primary goal of a modern interactive tool is that the tool always feels
/// responsive; a user will never wonder if the tool is locked up, or why things
/// are sluggish. The essentials were set up in Chapter 1. The user interface runs
/// on the main thread; an empty render engine, and an empty application state
/// engine were launched on separate threads, ready to do work.
/// 
/// The user interface will be kept as lightweight as possible, so that it is
/// always ready to respond to user activity. The strategy gusteau uses to accomplish
/// that is to ensure that the user interface thread does no work beyond displaying
/// the interface and responding to the user. The application context thread is
/// going to do actual work.
///
/// To accomplish this separation gusteau will use the principles explained by
/// C.A.R. Hoare, in the book *Communicating Sequential Processes* (CSP). The book was
/// published in 1985 by Prentice Hall International, and can be obtained online
/// at: <http://www.usingcsp.com/>.
///
/// CSP introduces a formal calculus of processes; its formalisms and structures 
/// are reflected in the design of modern concurrent languages  and libraries such 
/// as occam, libthread, Go, CHP, and more. The goals of CSP were to 
/// 
/// * describe interesting applications, 
/// * be efficient in implementation
/// * be useful in specification, design, and verification of systems.
/// 
/// CSP is a calculus of interaction.
/// 
/// CSP defines a process as the behavior pattern of an object; wherein events lead
/// to subsequent behaviors. Events are instantaneous actions, without causality.
/// They are not considered to be sent or received; they merely represent that 
/// something has occurred. An event may be thought of as a synchronization primitive
/// that processes may engage with.
/// 
/// A CSP process is denoted (E -> B), read as "E then B". The process upon 
/// receiving an event "E", engages in behavior "B".  The power of CSP comes from 
/// processes having an interesting recursive definition.
/// 
/// If a CLOCK behavior emits a single tick, a ticking clock can be defined as 
/// 
/// CLOCK = (tick -> CLOCK)
///
/// This can then be expanded via substitution as many times as we want. 
///
///    CLOCK = (tick -> (tick -> (tick -> (tick -> (tick -> CLOCK)))))
///
/// A tick tock clock would be
///
/// CLOCK = (tick -> (tock -> CLOCK))
///
/// tick and tock are CLOCK's alphabet.
///
/// Actions are considered to be instantaneous;
/// since timing can be considered independently. There's no concept of simultaneity;
/// if two events are considered to occur simultaneously, they are treated as a
/// single event.
///
/// Every process has an implicit STOP behavior. 
///
/// QUIT = (quit -> STOP)
///
/// If STOP is encountered, the process is finished.
///
/// Our events are going to be defined using a similar syntax, with a practical
/// extension; which is that our events will be adorned with a message to be
/// emitted whenever a process fires. A simple example follows, implementing the
/// two clock examples. 
///<C++
#include "csp.h"

void test_csp()
{
    ///>
    /// The syntax for events is very simple. 
    /// EVENT_NAME = PROCESS
    /// PROCESS = (event -> PROCESS OUTPUT)
    /// OUTPUT = "string"
    ///<C++
    char* csp_clock_sample_src = R"csp(
        CLOCK = (tick -> CLOCK "ticked")
        CLOCK2 = (tick -> (tock -> CLOCK2 "clock2_tocked") "ticked")
    )csp";

    CSP* csp_clock_sample = csp_parse(csp_clock_sample_src, strlen(csp_clock_sample_src));

    csp_bind_lambda(csp_clock_sample, "ticked", [](int){ printf("tick\n"); });
    csp_bind_lambda(csp_clock_sample, "clock2_tocked", [](int){ printf("tock\n"); });
    csp_emit(csp_clock_sample, "tick", {});
    csp_emit(csp_clock_sample, "foo", {});   // an event in an unknown alphabet
    csp_emit(csp_clock_sample, "tock", {});
    csp_emit(csp_clock_sample, "tick", {});
    csp_emit(csp_clock_sample, "tock", {});
    csp_update(csp_clock_sample);
}
///>

#ifdef GUSTEAU_chapter2

/// The application context is going to own the CSP processes.
/// All the things that application context can do will be registered here
/// as processes.
/// The quit event leads to STOP, and outputs a "join_now".
/// Each process can be thought of as an independent state machine, running
/// concurrently with all the others.
///
/// Some of these commands do nothing but respond, output, and wait to respond again
/// and as such their definition is a bit boring. When we get to more complex
/// programs, we'll see more interesting state machines than this.
///<C++
char* csp_ac_src = R"csp(
    APPEND_LINE = (append_line -> APPEND_LINE "append_line")
    POP_LINE = (pop_line -> POP_LINE "pop_line")
    QUIT = (quit -> STOP "join_now")
    TICK = (tick -> TICK "tick")
)csp";
///>
/// Processes will communicate via a blackboard. The main feature of the
/// the blackboard is that it is a threadsafe associative map of identifiers
/// to data. A process' action can request an id from the blackboard while
/// simultaneously writing a value to it, atomically.
///
#include "blackboard.h"
#include "journal.h"
#include <thread>

///<C++
class ApplicationContext : public ApplicationContextBase
{
public:	
    ApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui_) 
    : root_graphics_context(gc)
    , blackboard(new Blackboard())
    {
        ui = ui_;
        CreateCSP();
    }
    ///>
    /// Initialize all the CSP definitions
    ///<C++
    void CreateCSP()
    {
        csp = csp_parse(csp_ac_src, strlen(csp_ac_src));

        csp_bind_lambda(csp, "append_line", [this](int id)
        {
            if (id)
            {
                ///>
                /// The append line event comes with a string to append,
                /// so fetch the data from the blackboard, and coerce it to be a string.
                /// If that works, append it to the lines buffer.
                ///<C++
                TypedData* d = blackboard_get(blackboard, id);
                auto td = dynamic_cast<Data<std::string>*>(d);
                if (td)
                {
                    ///>
                    /// append line must append the line.
                    ///<C++
                    lines.push_back(td->value());
                    ///>
                    /// The operation modified the ApplicationContext, so record the operation
                    /// in the journal. The journal will come in handy later during the discussion
                    /// of undo and redo. For now, the journal merely records what happened.
                    /// If a journal where to be played forward on another ApplicationContext,
                    /// the other ApplicationContext should achieve the identical state of the 
                    /// first ApplicationContext. Of course, in a large or long-running
                    /// application, it might be impractical to record every action; this
                    /// too will be a topic for a later chapter.
                    ///<C++
                    journal.emplace_back(std::move(JournalEntry{"append_line", d}));
                }
                else
                {
                    ///>
                    /// The data from the blackboard is not reference counted
                    /// so it must be deleted here.
                    ///<C++
                    delete d;
                }
            }
        });
        csp_bind_lambda(csp, "pop_line", [this](int)
        {
            if (lines.size())
            {
                lines.pop_back();
                journal.emplace_back(std::move(JournalEntry{"pop_line", nullptr}));
            }
        });
        ///>
        /// Whenever a tick occurs; the variable count will be incremented.
        ///<C++
        csp_bind_lambda(csp, "tick", [this](int)
        {
            ++count;
            ///>
            /// Ticking is not an action that was initiated by the user and doesn't
            /// need to appear in a journal.
            ///<C++
        });
        ///>
        /// The join_now action is here, bound by name to the csp QUIT process.
        ///<C++
        csp_bind_lambda(csp, "join_now", [this](int) { join_now = true; });
        ///>

        /// A demonstration thread that runs at 10Hz, emitting ticks as it goes.
        /// This shows a very simple way that threads can communicate.
        ///<C++
        clock = std::thread([this]()
        {
            while (!join_now)
            {
                csp_emit(csp, "tick", 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    ///>
    /// Given a journal, replay that journal on the current context.
    ///<C++
    void ReplayJournal(const std::vector<JournalEntry>& journal)
    {
        for (auto& i : journal)
        {
            int id = 0;
            if (i.data)
                id = blackboard_new_entry(blackboard, i.data->clone());
            csp_emit(csp, i.name.c_str(), id); 
        }
    }

    ///>
    /// Writing a journal is straight forward
    ///<C++
    void SaveJounal(char const*const path)
    {
        if (!path)
            return;

        FILE* f = fopen(path, "wb");
        if (!f)
            return;

        ///>
        /// This example is simplistic, as the only journal data that there is
        /// to be saved is string data.
        ///<C++
        for (auto& j : journal)
            fprintf(f, "%s: %s\n", j.name.c_str(), j.data->to_string().c_str());

        fclose(f);
    }
    ///>
    /// As is reading one.
    ///<C++
    void LoadJournal(char const*const path)
    {
        if (!path)
            return;

        FILE* f = fopen(path, "rb");
        if (!f)
            return;

        std::vector<JournalEntry> j;

        const size_t sz = 1024;
        char buffer[sz];
        while (fgets(buffer, sz, f))
        {
            ///>
            /// This example is simplistic, as the only journal data that there is
            /// to be loaded is string data. A following chapter will generaliize
            /// this mechanism.
            ///<C++
            StrView b = lab::Text::Strip({buffer, sz});
            StrView data_str = lab::Text::ScanForCharacter(b, ':');
            std::string name{b.curr, size_t(data_str.curr - b.curr)};
            Data<std::string>* data = new Data<std::string>(std::string{data_str.curr, data_str.sz});
            j.emplace_back(std::move(JournalEntry(name, data)));
        }

        fclose(f);
        ReplayJournal(j);
    }
    ///>
    /// This version of the constructor also accepts a journal, and replays that
    /// journal on the ApplicationContext to reproduce the state of an original
    /// ApplicationContext. This kind of mechanism can be used for loading a
    /// saved state, reconstructing a state for regression or unit testing, 
    /// tutorials, and so on.
    ///<C++
    ApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui_, const std::vector<JournalEntry>& journal) 
    : root_graphics_context(gc)
    , blackboard(new Blackboard())
    {
        ui = ui_;
        CreateCSP();
        ReplayJournal(journal);
    }

    ~ApplicationContext()
    {
        ///>
        /// join_now is most likely true from having been set by a Quit
        /// event, but just to be sure, set it here.
        ///<C++
        join_now = true;

        ///>
        /// Allow the clock thread to join gracefully
        ///<C++
        clock.join();

        delete csp;
        delete blackboard;
    }


///>
/// The ApplicationContext will be updated once per framing round.
/// This is the place where any pending events will be applied to processes,
/// and process behaviors will run. The update occurs after the UI has been
/// rendered, in the UI thread. For this reason, bound lambdas need to be
/// written in consideration of UI responsiveness. They should only do 
/// lightweight work that does not interrupt the user experience.
/// What's going to happen is that events will be emitted from many threads,
/// they will be bottlenecked and serialized through the main UI thread, and
/// the main UI thread will call any bound lambdas which might optionally
/// dispatch new work to other threads.
///<C++
    virtual void Update() override
    {
        if (csp)
            csp_update(csp);
    }
///>

    GraphicsContext& root_graphics_context;
    StateContext state;
    RenderContext render;
    std::thread clock;

    int count = 0;
    std::vector<std::string> lines;

    CSP* csp = nullptr;
    Blackboard* blackboard = nullptr;

    std::vector<JournalEntry> journal;
};

std::shared_ptr<ApplicationContextBase> CreateApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui)
{
    return std::make_shared<ApplicationContext>(gc, ui);
}


class GusteauChapter2UI : public UIContext
{
public:
    GusteauChapter2UI(GraphicsContext& gc,
        const std::string& window_name, int width, int height)
        : UIContext(gc, window_name, width, height)
        {}

    virtual void Run(ApplicationContextBase& ac) override
    {
        auto ac_ptr = reinterpret_cast<ApplicationContext*>(&ac);

        ImGui::Text("Hello Chapter 2");
        if (ImGui::Button("Quit"))
        {
///>
/// The UI in chapter two uses the CSP mechanism to do it's work,
/// instead of setting variables on the ApplicationContext as it did previously.
///<C++
            csp_emit(ac_ptr->csp, "quit", 0);
        }
        static char tick_text[128];
        sprintf(tick_text, "tick: %d", ac_ptr->count);
        ImGui::TextUnformatted(tick_text);

        static char buff[256];
        ImGui::InputText("Line: ", buff, sizeof(buff));
        if (ImGui::Button("Append"))
        {
            int id = blackboard_new_entry(ac_ptr->blackboard, new Data<std::string>(std::string{buff}));
            csp_emit(ac_ptr->csp, "append_line", id);
        }
        if (ImGui::Button("Pop"))
        {
            csp_emit(ac_ptr->csp, "pop_line", 0);
        }
        ImGui::TextUnformatted("------ LINES ------");
        size_t sz = ac_ptr->lines.size();
        for (auto i = 0; i < sz; ++i)
        {
            ImGui::TextUnformatted(ac_ptr->lines[i].c_str());
        }
    }
};
///>

std::shared_ptr<UIContext> CreateUIContext(GraphicsContext& gc)
{
    auto ui = std::make_shared<GusteauChapter2UI>(gc, "gusteau", 1024, 1024);
    return ui;
}

#endif // GUSTEAU_chapter2
