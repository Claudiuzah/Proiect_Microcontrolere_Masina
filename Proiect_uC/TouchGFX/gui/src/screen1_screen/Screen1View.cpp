#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::updateDistanceView(uint16_t left, uint16_t front, uint16_t right)
{
    // Actualizează buffer-ele de text pentru senzorii VL53L0X
    Unicode::snprintf(Stare_Senzor_LBuffer, STARE_SENZOR_L_SIZE, "%d", left);
    Unicode::snprintf(Stare_Senzor_FBuffer, STARE_SENZOR_F_SIZE, "%d", front);
    Unicode::snprintf(Stare_Senzor_RBuffer, STARE_SENZOR_R_SIZE, "%d", right);

    // Redesenează widget-urile pe ecran
    Stare_Senzor_L.invalidate();
    Stare_Senzor_F.invalidate();
    Stare_Senzor_R.invalidate();
}