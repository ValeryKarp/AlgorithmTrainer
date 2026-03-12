#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "CoreAPI.h"
#include "Task.h"

namespace py = pybind11;
using namespace algotrainer;

PYBIND11_MODULE(core, m) {
    m.doc() = "Algorithm trainer core module";

    py::class_<Task>(m, "Task")
        .def_readonly("id", &Task::id)
        .def_readonly("topic", &Task::topic)
        .def_readonly("difficulty", &Task::difficulty)
        .def_readonly("question", &Task::question)
        .def_readonly("explanation", &Task::explanation)
        .def_readonly("image_path", &Task::image_path)
        .def_readonly("source", &Task::source)
        .def_readonly("title", &Task::title);

    py::class_<CoreAPI>(m, "CoreAPI")
        .def(py::init<const std::string&>())
        .def("get_next_task", &CoreAPI::getNextTask)
        .def("submit_result", &CoreAPI::submitResult)
        .def("get_stats", &CoreAPI::getStats)
        .def("reset_progress", &CoreAPI::resetProgress)
        .def("set_enabled_topics", &CoreAPI::setEnabledTopics)
        .def("get_all_tasks", &CoreAPI::getAllTasksByTopic)   // новый метод
        .def("get_task_by_id", &CoreAPI::getTaskById)         // новый метод
        .def("shutdown", &CoreAPI::shutdown);
}