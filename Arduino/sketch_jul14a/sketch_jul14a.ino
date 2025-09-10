#include <Mouse.h>
#include <Keyboard.h>

String input = "";

void setup() {
  Serial.begin(9600);
  Mouse.begin();
  Keyboard.begin();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processCommand(input);
      input = "";
    } else {
      input += c;
    }
  }
}

void processCommand(String cmd) {
  cmd.trim(); // <<< 关键！去掉 \r\n

  if (cmd == "leftclick") {
    Mouse.click(MOUSE_LEFT);
    Serial.println("Left click triggered.");
  } else if (cmd == "rightclick") {
    Mouse.click(MOUSE_RIGHT);
    Serial.println("Right click triggered.");
  } else if (cmd.startsWith("move:")) {
    int comma = cmd.indexOf(',');
    if (comma == -1) {
      Serial.println("Invalid move command format.");
      return;
    }
    int dx = cmd.substring(5, comma).toInt();
    int dy = cmd.substring(comma + 1).toInt();
    smoothMove(dx, dy);
    delay(500);
  } else if (cmd.startsWith("keypress:")) {
    if (cmd.length() <= 9) {
      Serial.println("Invalid keypress command.");
      return;
    }
    char key = cmd.charAt(9);
    Keyboard.press(key);
    delay(100);  // 模拟按下时间
    Keyboard.release(key);
    Serial.print("Keypress triggered: ");
    Serial.println(key);
  } else if (cmd == "rightdoubleclick") {
    Mouse.click(MOUSE_LEFT);
    delay(50);  // 两次点击之间的间隔，视游戏容忍度调整
    Mouse.click(MOUSE_LEFT);
    Serial.println("Double click triggered");  
  } else if (cmd.startsWith("drag:")) {
    int comma = cmd.indexOf(',');
    if (comma == -1) {
      Serial.println("Invalid drag command format.");
      return;
    }
    int dx = cmd.substring(5, comma).toInt();
    int dy = cmd.substring(comma + 1).toInt();

    Mouse.press(MOUSE_LEFT);      // 按下鼠标
    smoothMove(dx, dy);           // 拖动
    delay(100);                   // 稳定
    Mouse.release(MOUSE_LEFT);    // 放开鼠标

    Serial.print("Dragged by: ");
    Serial.print(dx);
    Serial.print(", ");
    Serial.println(dy);
  } else {
    Serial.print("Unknown command: ");
    Serial.println(cmd);
  }
}

void smoothMove(int dx, int dy) {
  int absDx = abs(dx);
  int absDy = abs(dy);

  // 总步数：越远移动越多步，越近越快
  int steps = max(absDx, absDy) / 2; // 速度翻倍（2 倍步长）
  steps = max(1, steps); // 防止除 0

  if (steps == 0) return;

  float stepX = (float)dx / steps;
  float stepY = (float)dy / steps;

  float accumX = 0, accumY = 0;

  for (int i = 0; i < steps; i++) {
    accumX += stepX;
    accumY += stepY;

    int moveX = round(accumX);
    int moveY = round(accumY);

    accumX -= moveX;
    accumY -= moveY;

    Mouse.move(moveX, moveY);

    // 延迟更精细，用微秒控制速度（推荐在 1000~5000 微秒之间）
    delayMicroseconds(1000);  // 3 毫秒


  // ✅ 最后一步再精确校正
  int finalMoveX = round(accumX);
  int finalMoveY = round(accumY);
  if (finalMoveX != 0 || finalMoveY != 0)
    Mouse.move(finalMoveX, finalMoveY);
  }
}