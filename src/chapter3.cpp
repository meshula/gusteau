
/// Chapter 3 - Undo and Redo
///
/// The build of Chapter 3 will be similar to that of Chapter 1.
///
/// ~~~~
/// stddoc < chapter3.cpp > chapter3.html
/// mkdir gusteau-build-chapter3
/// cd gusteau-build-chapter3
/// cmake ../gusteau -DCHAPTER=chapter3 -G "generator"
/// ~~~~
/// where "generator" might be "Visual Studio 15 2017 Win64" for Visual Studio,
/// "Xcode" for Xcode, or "Make" for Make.
///
/// When CMake completes, build and run the resulting project.
///
/// The structure of Chapter 3 is very similar to that of Chapter 1.
/// Communicating Sequential Process (CSP) techniques will allow the user
/// interface to modify the state of the application context. The DSL
/// introduced in Chapter 2 will be used here as well.
///
#define LABTEXT_ODR
#include "LabText.h"
#undef LABTEXT_ODR
#include "chapter1.cpp"
#include "csp.h"
#include "blackboard.h"
#include "journal.h"
#include <thread>
///
/// Typical mechanisms for undo and redo will be illustrated via an application
/// that functions like a simple calculator.
/// First, the actions the calculator can perform are declared in CSP. Once
/// again, the behavior of the state machines are very simple, so the declarations
/// follow the most straight forward form.
///<C++
char* csp_ac_src = R"csp(
    PUSH_VALUE = (push_value -> PUSH_VALUE "push_value")
    POP_VALUE = (pop_value -> POP_VALUE "pop_value")
    ADD = (add -> ADD "add")
    SUBTRACT = (subtract -> SUBTRACT "subtract")
    MULTIPLY = (multiply -> MULTIPLY "multiply")
    DIVIDE = (divide -> DIVIDE "divide")
    QUIT = (quit -> STOP "join_now")
)csp";

class ApplicationContext : public ApplicationContextBase
{
public: 
    ApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui_) 
    : root_graphics_context(gc)
    , blackboard(new Blackboard())
    {
        ui = ui_;
    }

    ///>
    /// The initialize is split out from the constructor because
    /// the lambdas will need to create shared pointers to the 
    /// ApplicationContext. It's impossible from the constructor
    /// because the shared_pointer won't have been initialized yet.
    virtual void Init() override
    {
        CreateCSP();
    }

    ///>
    /// Initialize all the CSP definitions
    ///<C++
    void CreateCSP()
    {
        csp = csp_parse(nullptr, csp_ac_src, strlen(csp_ac_src));

        /// The execution of actions becomes complicated by the introduction of
        /// undo. The first consideration is that we mustn't keep references to
        /// the application context in all the history's lambdas
        std::shared_ptr<ApplicationContext> app = std::dynamic_pointer_cast<ApplicationContext>(this->shared_from_this());

        csp_bind_lambda(csp, "push_value", [app](int id)
        {
            if (id && app)
            {
                ///>
                /// The append line event comes with a floating point value 
                ///<C++
                TypedData* d = blackboard_get(app->blackboard, id);
                auto td = dynamic_cast<Data<float>*>(d);
                if (td)
                {
///>
/// The operation modified the ApplicationContext, so record the operation
/// in the journal. The journal records both the action that was taken, 
/// and the reverse operation. The inverse of pushing a value on the stack is to pop it.
/// Previously, the journal was a simple record of an action.  Now,
/// we are going to explore more complex actions, with more complex undo
/// behavior. So a more sophisticated journal is required. Since the application
/// is multi-threaded, many actions could occur concurrently. In the case of
/// an action with a simple undo, that's no big deal, since one action is paired
/// with one undo action. In a moment though, we will see an undo action that
/// requires multiple steps that have to go together. We introduce therefore
/// a Journal::Transaction that records a single atomic history/undo pair.
///<C++
                    Journal::Transaction transaction 
                    {  "push_value", 
                        [app, td]() 
                        {
                            app->value_stack.push_back(td->value());
                        },
                        [app]() 
                        { 
                            app->value_stack.pop_back(); 
                        }
                    };
                    transaction.action();
                    app->journal.commit(std::move(transaction));
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
        csp_bind_lambda(csp, "pop_value", [app](int)
        {
            if (app->value_stack.size())
            {
                // remember the value that was at the back of the value stack.
                float value = *app->value_stack.rbegin();

                /// The inverse of popping a value is to push it again.
                Journal::Transaction transaction
                {
                    "pop_value",
                    [value, app]()
                    {
                        app->value_stack.pop_back();
                    },
                    [value, app]()
                    {
                        app->value_stack.push_back(value);
                    }
                };
                transaction.action();
                app->journal.commit(std::move(transaction));
            }
        });
        csp_bind_lambda(csp, "add", [app](int)
        {
            ///>
            /// This application is very simple, and doesn't report problems
            /// such as not enough values on the stack. A real application would.
            ///<C++
            if (app->value_stack.size() >= 2)
            {
                auto it = app->value_stack.rbegin();
                float value2 = *it++;
                float value1 = *it++;

                /// the history needs only record that an add occurred.
                /// The undo must remove the result, and push the original values.
                /// This is where the transactional nature of the journal comes
                /// into play.
                Journal::Transaction transaction
                {
                    "add",
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1 + value2);
                    },
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1);
                        app->value_stack.emplace_back(value2);
                    }
                };
                transaction.action();
                app->journal.commit(std::move(transaction));
            }
        });
        csp_bind_lambda(csp, "subtract", [app](int)
        {
            if (app->value_stack.size() >= 2)
            {
                auto it = app->value_stack.rbegin();
                float value2 = *it++;
                float value1 = *it++;

                Journal::Transaction transaction
                {
                    "subtract",
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1 - value2);
                    },
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1);
                        app->value_stack.emplace_back(value2);
                    }
                };
                transaction.action();
                app->journal.commit(std::move(transaction));
            }
        });
        csp_bind_lambda(csp, "multiply", [app](int)
        {
                auto it = app->value_stack.rbegin();
                float value2 = *it++;
                float value1 = *it++;

                Journal::Transaction transaction
                {
                    "multiply",
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1 * value2);
                    },
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1);
                        app->value_stack.emplace_back(value2);
                    }
                };
                transaction.action();
                app->journal.commit(std::move(transaction));
        });
        csp_bind_lambda(csp, "divide", [app](int)
        {
            if (app->value_stack.size() >= 2)
            {
                auto it = app->value_stack.rbegin();
                float value2 = *it++;
                float value1 = *it++;

                Journal::Transaction transaction
                {
                    "divide",
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1 / value2);
                    },
                    [app, value1, value2]() 
                    {
                        app->value_stack.pop_back();
                        app->value_stack.emplace_back(value1);
                        app->value_stack.emplace_back(value2);
                    }
                };
                transaction.action();
                app->journal.commit(std::move(transaction));
            }
        });
        ///>
        /// The join_now action is here, bound by name to the csp QUIT process.
        ///<C++
        csp_bind_lambda(csp, "join_now", [this](int) { join_now = true; });
    }

    ~ApplicationContext()
    {
        ///>
        /// join_now is most likely true from having been set by a Quit
        /// event, but just to be sure, set it here.
        ///<C++
        join_now = true;

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

    int count = 0;
    std::vector<float> value_stack;

    CSP* csp = nullptr;
    Blackboard* blackboard = nullptr;

    Journal journal;
};

std::shared_ptr<ApplicationContextBase> CreateApplicationContext(GraphicsContext& gc, std::shared_ptr<UIContext> ui)
{
    return std::make_shared<ApplicationContext>(gc, ui);
}


class GusteauChapter3UI : public UIContext
{
public:
    GusteauChapter3UI(GraphicsContext& gc,
        const std::string& window_name, int width, int height)
        : UIContext(gc, window_name, width, height)
        {}

    virtual void Run(ApplicationContextBase& ac) override
    {
        auto app = reinterpret_cast<ApplicationContext*>(&ac);

        ImGui::Text("Hello Chapter 3");
        if (ImGui::Button("Quit"))
        {
            csp_emit(app->csp, "quit", 0);
        }


        static char buff[256];
        ImGui::InputText("###value", buff, sizeof(buff));
        if (ImGui::Button("Push"))
        {
            float v = static_cast<float>(atof(buff));
            int id = blackboard_new_entry(app->blackboard, new Data<float>(v));
            csp_emit(app->csp, "push_value", id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Pop"))
            csp_emit(app->csp, "pop_value", 0);

        if (ImGui::Button("+"))
            csp_emit(app->csp, "add", 0);
        ImGui::SameLine();
        if (ImGui::Button("-"))
            csp_emit(app->csp, "subtract", 0);
        ImGui::SameLine();
        if (ImGui::Button("*"))
            csp_emit(app->csp, "multiply", 0);
        ImGui::SameLine();
        if (ImGui::Button("/"))
            csp_emit(app->csp, "divide", 0);

        if (ImGui::Button("Undo"))
        {
            app->journal.undo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Redo"))
        {
            app->journal.redo();
        }

        ImGui::TextUnformatted("------ STACK ------");
        size_t sz = app->value_stack.size();
        for (auto i = 0; i < sz; ++i)
        {
            char buff[256];
            sprintf(buff, "%f", app->value_stack[i]);
            ImGui::TextUnformatted(buff);
        }
    }
};
///>

std::shared_ptr<UIContext> CreateUIContext(GraphicsContext& gc)
{
    auto ui = std::make_shared<GusteauChapter3UI>(gc, "gusteau", 1024, 1024);
    return ui;
}
