
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
/// Our events are going to be defined using a similar syntax, with a practical
/// extension; which is that our events will be adorned with a message to be
/// emitted whenever a process fires. A simple example follows, implementing the
/// two clock examples. 
///<C++
#include "csp.h"
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
csp_update(csp);
///>
