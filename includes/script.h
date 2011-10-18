/**
 * Scripting Helper
 */

#ifndef SCRIPTING_H__
#define SCRIPTING_H__

BOOL SCRIPTING_Init();
BOOL SCRIPTING_Destory();
BOOL SCRIPTING_AfterLoaded();
void SCRIPTING_Update(float);

void SCRIPTING_Key(unsigned long keySym);
void SCRIPTING_Touch(char buttonNum, int x, int y);
void SCRIPTING_Move(int dX, int dY);

#endif
