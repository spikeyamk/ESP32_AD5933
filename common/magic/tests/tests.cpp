#include "magic/commands/serializer.hpp"
#include "magic/commands/deserializer.hpp"
#include "magic/results/serializer.hpp"
#include "magic/results/deserializer.hpp"

#include "magic/tests.hpp"

namespace Magic {
    namespace Tests {
        template<typename Serializer, typename Deserializer>
        struct Runner {
            template<typename ... Args>
            struct InnerRunner {
                template<typename HeadArg, typename ... TailArgs>
                static int inner_run() {
                    const HeadArg obj {};
                    const auto ser { Serializer::run(obj) };
                    const auto de { Deserializer::run<HeadArg>(ser) };

                    if(boost::pfr::eq(obj, de) == false) {
                        return -1 * static_cast<int>(sizeof...(TailArgs) + 1);
                    }

                    if constexpr(sizeof...(TailArgs) != 0) {
                        return inner_run<TailArgs...>();
                    }
                    return 0;
                }

                static int run() {
                    return inner_run<Args...>();
                }
            };
        };

        int commands_with_serde() {
            using CommandsRunner = Commands::Pack::apply_to<Runner<Commands::Serializer, Commands::Deserializer>::InnerRunner>;
            return CommandsRunner::run();
        }
        
        int results_with_serde() {
            using ResultsRunner = Results::Pack::apply_to<Runner<Results::Serializer, Results::Deserializer>::InnerRunner>;
            return ResultsRunner::run();
        }
    }
}