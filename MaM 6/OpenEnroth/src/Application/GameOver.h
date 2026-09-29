#pragma once

class GraphicsImage;

void GameOver_Setup();
/**
 * @param lost                          MM6 only, the ending where the reactor destroys the world.
 * @return                              Picture of the final screen.
 */
GraphicsImage *CreateWinnerCertificate(bool lost = false);
