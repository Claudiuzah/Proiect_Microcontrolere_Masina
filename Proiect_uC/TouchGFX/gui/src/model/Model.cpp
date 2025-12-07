#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <stdint.h>

// Declarăm că variabilele există în alt fișier (main.c)
extern "C" {
    extern volatile uint16_t global_dist_left;
    extern volatile uint16_t global_dist_front;
    extern volatile uint16_t global_dist_right;
}

Model::Model() : modelListener(0) {}

void Model::tick()
{
    // Această funcție se apelează la fiecare frame (60Hz)
    // Trimitem datele către Presenter
    if (modelListener != 0)
    {
        modelListener->updateSensors(global_dist_left, global_dist_front, global_dist_right);
    }
}