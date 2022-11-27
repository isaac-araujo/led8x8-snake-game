#include <LedControl.h>
#include <stdlib.h>
#include <stdio.h>

#define DIN 11
#define CS 12
#define CLK 13
#define JOY 2

#define VRX_PIN  A0 // Arduino pin connected to VRX pin
#define VRY_PIN  A1 // Arduino pin connected to VRY pin

#define LEFT_THRESHOLD  400
#define RIGHT_THRESHOLD 800
#define UP_THRESHOLD    400
#define DOWN_THRESHOLD  800

#define COMMAND_NO 0x00
#define COMMAND_UP 0x01
#define COMMAND_DOWN 0x02
#define COMMAND_RIGHT 0x04
#define COMMAND_LEFT 0x08

#define WIN 4

int xValue = 0 ; // To store value of the X axis
int yValue = 0 ; // To store value of the Y axis
int command = COMMAND_NO;
unsigned long time;

int x = 5;
int y = 3;
int xActual = x;
int yActual = y;
int xFood = 1;
int yFood = 1;
int yNewFood;
int xNewFood;
int last = 4;
bool moved= false;
int delayTime = 500;
int delayInput;
int lenghtSnake;

LedControl lc = LedControl(DIN, CLK, CS, 0);

int mtx[8][8]={};

struct node
{
  int x;
  int y;
  struct node *next;
};
struct node *start=NULL;

void setup() {
  Serial.begin(9600);
  // Seleção de Dificuldade
  while (!Serial.available());
  delayInput = Serial.read();
  delayTime = delayTime/(delayInput%4+1);

  // Inicializando LED
  lc.shutdown(0, false);
  lc.setIntensity(0, 10); //max = 15
  lc.clearDisplay(0);

  // Inicializando cobra matrix
  mtx[y][x] = 1;
  mtx[y][x+1] = 1;
  mtx[y][x+2] = 1;
  lenghtSnake = 3;

  // Inicializando comida matrix
  mtx[yFood][xFood] = 2;

  // Inicializando Lista Encadeada
  create(x, y);
  insert_end(x+1, y);
  insert_end(x+2, y);
  
  showMatrix();
  displayLed();
  
  // Mostra comida
  lc.setLed(0, xFood, yFood, 1);

}

void loop() {
  // Le valores analogicos X e Y
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);
  moved = false;

  // converts the analog value to commands
  // reset commands
  command = COMMAND_NO;

  // Verifica comandos
  if (xValue < LEFT_THRESHOLD)
    command = command | COMMAND_UP;
  else if (xValue > RIGHT_THRESHOLD)
    command = command | COMMAND_DOWN;
  if (yValue < UP_THRESHOLD)
    command = command | COMMAND_RIGHT;
  else if (yValue > DOWN_THRESHOLD)
    command = command | COMMAND_LEFT;

  //Seleciona comandos  
  if (command & COMMAND_UP) {
    Serial.println("COMMAND UP");
    if (!(start->next->x == xActual && start->next->y == yActual+1)){
      yActual++;
      last = 1;
      move(xActual, yActual);
      moved = true;  
    }
    
  }
  else if (command & COMMAND_DOWN) {
    Serial.println("COMMAND DOWN");
    if (!(start->next->x == xActual && start->next->y == yActual-1)){
      yActual--;
      last = 2;
      move(xActual, yActual);
      moved = true;  
    }
  }

  else if (command & COMMAND_RIGHT) {
    Serial.println("COMMAND RIGHT");
    if (!(start->next->x == xActual+1 && start->next->y == yActual)){
      xActual++;
      last = 3;
      move(xActual, yActual);
      moved = true;      
    }
  }

  else if (command & COMMAND_LEFT) {
    Serial.println("COMMAND LEFT");
    if (!(start->next->x == xActual-1 && start->next->y == yActual)){
      xActual--;
      last = 4;
      move(xActual, yActual);
      moved = true;  
    }
  }
  
  // Executa ultimo comando se não tiver ação do jogador
  if (!moved){
    // continua movimento
    switch(last){
      case 1:
        yActual++;
        break;
      case 2:
        yActual--;
        break;
      case 3:
        xActual++;
        break;
      case 4:
        xActual--;
        break;
    }
    move(xActual, yActual);
  }

  // Diferencia comida piscando
  lc.setLed(0, xFood, yFood, 0);
  delay(29);
  lc.setLed(0, xFood, yFood, 1);


  delay(delayTime);
}

void(* resetFunc) (void) = 0;

void move(int x, int y){
  if (x < 0){
    x = 7;
  }
  if (y < 0){
    y = 7;
  }
  x = x % 8;
  y = y % 8;
  if(!eat(x, y)){
    delete_end();    
  }
  if(mtx[y][x]==1){  
    Serial.println("PERDEU");
    lostLED1();
    resetFunc();
  }
  
  insert_begin(x, y);
  xActual = x;
  yActual = y;
  // showMatrix();
  
}

bool eat(int x, int y)
{
  // verifica se comeu  
  if(x == xFood && y == yFood){
    lc.setLed(0, xFood, yFood, 0);
    lenghtSnake++;
    if (lenghtSnake == WIN){
      winLED();
    }
    
  // Nova posição comida     
    do{
      time = millis();
      xNewFood = time % 8;
      yNewFood = (time * 7) % 8;
      Serial.println("NOVA COMIDA");
    }while(mtx[yNewFood][xNewFood] == 1 || (yFood == yNewFood && xFood == xNewFood));

    yFood = yNewFood;
    xFood = xNewFood;

    lc.setLed(0, xFood, yFood, 1);
    mtx[yFood][xFood] = 2;
    return true;
  }
    return false;
}

void winLED(){
  byte smile[8]={0x3c, 0x42, 0x95, 0xa1, 0xa1, 0x95, 0x42, 0x3c};
  int i = 0;
  for(i=0;i<9;i++)
  {
    lc.setRow(0,i,smile[i]);
  }
  delay(10000);
  resetFunc();
}

void lostLED1(){
  byte bHeart[8]={0x1c, 0x3e, 0x7e, 0xfc, 0xfc, 0x7e, 0x3e, 0x1c};
  int i = 0;
  for(i=0;i<9;i++)
  {
    lc.setRow(0,i,bHeart[i]);
  }
  
  delay(1000);
  lc.setLed(0, 4, 0, 0);
  delay(500);
  lc.setLed(0, 3, 1, 0);
  delay(500);
  lc.setLed(0, 3, 2, 0);
  delay(500);
  lc.setLed(0, 4, 3, 0);
  delay(500);
  lc.setLed(0, 4, 4, 0);
  delay(500);
  lc.setLed(0, 3, 5, 0);
  delay(500);
  lc.setLed(0, 3, 6, 0);
  delay(10000);
  resetFunc();  
}

void lostLED2(){
  byte bHeart[8]={0x1c, 0x3e, 0x7e, 0xfc, 0xfc, 0x7e, 0x3e, 0x1c};
  int i = 0;
  for(i=0;i<9;i++)
  {
    delay(700);
    lc.setRow(0,i,bHeart[i]%1);
  }
  resetFunc(); 
}



void showMatrix()
{
  for(int i=7; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(mtx[i][j]);
      Serial.print(" ");
    }
    Serial.print("\n"); // new line
  }

  Serial.print("\n\n");
}

void create(int x, int y)
{
  struct node *temp,*ptr;
  temp=(struct node *)malloc(sizeof(struct node));
  if(temp==NULL)
  {
    Serial.println("\nOut of Memory Space:\n");
    exit(0);
  }
  temp->x = x;
  temp->y = y;
  temp->next=NULL;
  if(start==NULL)
  {
    start=temp;
  }
  else
  {
    ptr=start;
    while(ptr->next!=NULL)
    {
      ptr=ptr->next;
    }
    ptr->next=temp;
  }
}

void displayLed()
{
  struct node *ptr;
  lc.clearDisplay(0);
  if(start==NULL)
  {
    Serial.println("\nList is empty:\n");
    return;
  }
  else
  {
    ptr=start;
    while(ptr!=NULL)
    {
      lc.setLed(0, ptr->x, ptr->y, true);
      ptr=ptr->next ;
    }
  }
}

void insert_begin(int x, int y)
{
  mtx[y][x] = 1;
  struct node *temp;
  temp=(struct node *)malloc(sizeof(struct node));
  if(temp==NULL)
  {
    Serial.println("\nOut of Memory Space\n");
    return;
  }
  temp->x = x;
  temp->y = y;
  temp->next =NULL;
  if(start==NULL)
  {
    start=temp;
  }
  else
  {
    temp->next=start;
    lc.setLed(0, temp->x, temp->y, 1);
    start=temp;
  }
}

void delete_end()
{
  struct node *temp,*ptr;
  if(start==NULL)
  {
          Serial.println("\nList is Empty:");
          exit(0);
  }
  else if(start->next ==NULL)
  {
    ptr=start;
    start=NULL;
    // Serial.println("\nThe deleted element is:%d,%d\t",ptr->x,ptr->y);
    free(ptr);
  }
  else
  {
    ptr=start;
    while(ptr->next!=NULL)
    {
      temp=ptr;
      ptr=ptr->next;
    }
    mtx[ptr->y][ptr->x] = 0;
    lc.setLed(0, ptr->x, ptr->y, 0);
    temp->next=NULL;
    // Serial.println("\nThe deleted element is:%d,%d\t",ptr->x,ptr->y);
    free(ptr);
  }
}

void insert_end(int x, int y)
{
  struct node *temp,*ptr;
  temp=(struct node *)malloc(sizeof(struct node));
  if(temp==NULL)
  {
    Serial.println("\nOut of Memory Space:\n");
    return;
  }
  temp->x = x;
  temp->y = y;
  temp->next =NULL;
  if(start==NULL)
  {
    start=temp;
  }
  else
  {
    ptr=start;
    while(ptr->next !=NULL)
    {
      ptr=ptr->next ;
    }
    ptr->next = temp;
  }
}
