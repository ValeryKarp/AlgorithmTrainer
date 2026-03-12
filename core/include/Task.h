#pragma once
#include <string>

namespace algotrainer {

    struct Task {
        int id;
        std::string title;
        std::string topic;
        int difficulty; // 1..10
        std::string question;
        std::string explanation;
        std::string image_path;
        std::string source;

        Task() = default;
        Task(int id, std::string topic, int difficulty,
            std::string question, std::string explanation,
            std::string image_path = "", std::string source = "", std::string title = "")
            : id(id), topic(std::move(topic)), difficulty(difficulty),
                question(std::move(question)), explanation(std::move(explanation)),
                image_path(std::move(image_path)), source(std::move(source)),
                title(std::move(title)) {}
    };

} // namespace algotrainer