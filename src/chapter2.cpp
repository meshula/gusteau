
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
    char* csp_clock_sample_src = R"csp(
        CLOCK = (tick -> CLOCK "ticked")
        CLOCK2 = (tick -> (tock -> CLOCK2 "clock2_tocked") "ticked")
    )csp";

    CSP* csp_clock_sample = csp_parse(csp_clock_sample_src, strlen(csp_clock_sample_src));

    csp_bind_lambda(csp_clock_sample, "ticked", [](){printf("tick\n");});
    csp_bind_lambda(csp_clock_sample, "clock2_tocked", [](){printf("tock\n");});
    csp_emit(csp_clock_sample, "tick");
    csp_emit(csp_clock_sample, "foo");   // an event in an unknown alphabet
    csp_emit(csp_clock_sample, "tock");
    csp_emit(csp_clock_sample, "tick");
    csp_emit(csp_clock_sample, "tock");
    csp_update(csp_clock_sample);
}
///>

#ifdef GUSTEAU_chapter2

/// The application context is going to own the CSP processes.
/// All the things that application context can do will be registered here
/// as processes.
/// The quit event leads to STOP, and outputs a "join_now".
char* csp_ac_src = R"csp(
    QUIT = (quit -> STOP "join_now")
)csp";

///<C++
class ApplicationContext : public ApplicationContextBase
{
public:	
    ApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui_) 
        : root_graphics_context(gc)
    {
        ui = ui_;
        csp = csp_parse(csp_ac_src, strlen(csp_ac_src));
        auto self = this;
        csp_bind_lambda(csp, "join_now", [self]() { self->join_now = true; });
    }

    ~ApplicationContext()
    {
        delete csp;
    }

///>
/// The ApplicationContext will be updated, once per framing round.
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

    CSP* csp = nullptr;
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
/// The UI in chapter two is going to utilize the CSP mechanism to do it's work,
/// instead of setting variables on the ApplicationContext as it did previously.
///<C++
            csp_emit(ac_ptr->csp, "quit");
        }
    }
};
///>

std::shared_ptr<UIContext> CreateUIContext(GraphicsContext& gc)
{
    auto ui = std::make_shared<GusteauChapter2UI>(gc, "gusteau", 1024, 1024);
    return ui;
}

#endif
