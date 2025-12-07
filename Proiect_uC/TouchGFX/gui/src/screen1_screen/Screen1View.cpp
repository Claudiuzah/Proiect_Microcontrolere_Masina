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
    // Actualizează buffer-ul de text (Unicode::snprintf)
    Unicode::snprintf(textLeftBuffer, TEXTLEFT_SIZE, "%d", left);
    Unicode::snprintf(textFrontBuffer, TEXTFRONT_SIZE, "%d", front);
    Unicode::snprintf(textRightBuffer, TEXTRIGHT_SIZE, "%d", right);

    // Redesenează widget-urile
    textLeft.invalidate();
    textFront.invalidate();
    textRight.invalidate();
}