# MimiClaw 二次开发傻瓜指南 (小白向) 🐾

想要让 MimiClaw 控制你的硬件（比如点亮 LED、转动舵机、读取温湿度）？其实非常简单。本指南将教你如何通过 **添加工具 (Tool)** 和 **编写技能 (Skill)** 来实现。

---

## 1. 核心概念：工具 vs 技能

在 MimiClaw 中，它们就像“手”和“大脑”的关系：

- **工具 (Tool)**: 具体的 **C 代码实现**。它是 MimiClaw 的“手”，负责直接操作硬件（如 GPIO、I2C）。
- **技能 (Skill)**: 描述性的 **Markdown 文档**。它是 MimiClaw 的“大脑记忆”，告诉 AI 在什么情况下该调用哪个工具，以及调用的逻辑。

---

## 2. 第一步：添加一个硬件工具 (C 语言)

假设我们要添加一个控制蜂鸣器的工具 `buzzer_set`。

### 1. 创建工具文件
在 `main/tools/` 目录下创建 `tool_buzzer.c` 和 `tool_buzzer.h`。

**tool_buzzer.h**:
```c
#pragma once
#include "esp_err.h"
#include <stddef.h>

// 工具的执行函数，格式是固定的
esp_err_t tool_buzzer_execute(const char *input_json, char *output, size_t output_size);
```

**tool_buzzer.c**:
```c
#include "tool_buzzer.h"
#include "cJSON.h"
#include "driver/gpio.h" // ESP32 硬件驱动

#define BUZZER_GPIO 18 // 假设蜂鸣器接在 GPIO 18

esp_err_t tool_buzzer_execute(const char *input_json, char *output, size_t output_size) {
    cJSON *root = cJSON_Parse(input_json);
    cJSON *state = cJSON_GetObjectItem(root, "state");
    
    if (state && cJSON_IsNumber(state)) {
        int val = state->valueint;
        gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(BUZZER_GPIO, val); // 0 为灭，1 为响
        
        snprintf(output, output_size, "蜂鸣器状态已设置为: %s", val ? "开启" : "关闭");
    }
    
    cJSON_Delete(root);
    return ESP_OK;
}
```

### 2. 注册工具
打开 [tool_registry.c](file:///home/yanhuan/esp/mimiclaw/main/tools/tool_registry.c)，按照以下三步操作：

1. **包含头文件**:
   ```c
   #include "tools/tool_buzzer.h"
   ```
2. **定义工具信息**:
   在 `tool_registry_init` 函数中添加：
   ```c
   mimi_tool_t bz = {
       .name = "buzzer_set", // AI 看到的工具名
       .description = "控制蜂鸣器开关。参数 state: 1 开启，0 关闭。",
       .input_schema_json = "{\"type\":\"object\",\"properties\":{\"state\":{\"type\":\"integer\"}}}",
       .execute = tool_buzzer_execute,
   };
   register_tool(&bz);
   ```

---

## 3. 第二步：编写技能文件 (Markdown)

工具写好了，但 AI 还不熟悉怎么“聪明地”用它。我们需要在 `/spiffs/skills/` 下创建一个 `.md` 文件。

**文件：`/spiffs/skills/alarm.md`**
```markdown
# 闹钟与警报技能

当你需要提醒用户或发生紧急情况时，可以使用蜂鸣器。

## 使用逻辑
1. 如果用户说“帮我定个闹钟”，你可以结合 `cron_add` 定时任务，到时间后执行消息“触发警报音”。
2. 当收到“触发警报音”消息时，调用 `buzzer_set` 工具，state 设为 1。
3. 持续 2 秒后（或通过定时任务），再次调用 `buzzer_set` 将 state 设为 0。
```

> **小白贴士**：技能文件不需要写代码，就像写笔记一样。AI 会在对话前读取这些 `.md` 文件，从而学会如何组合使用工具。

---

## 4. 第三步：编译并烧录

1. **添加文件到工程**:
   修改 `main/CMakeLists.txt`，确保你新建的 `.c` 文件被包含在内（通常目录下所有 `.c` 会自动包含，除非你手动指定了列表）。
2. **编译**:
   ```bash
   idf.py build
   ```
3. **烧录**:
   ```bash
   idf.py flash
   ```

---

## 5. 进阶：如何让它更聪明？

- **读取硬件状态**：你可以写一个工具 `get_sensor_data`，让 AI 能看到环境温度。
- **ReAct 循环**：MimiClaw 会自动进行“思考-行动-观察”。比如你让它“如果温度高于30度就开风扇”，它会先调用温度工具，根据结果决定是否调用风扇工具。

---

## 常见问题 (FAQ)

- **Q: 为什么我改了代码 AI 还是不知道新工具？**
  A: 检查 `tool_registry.c` 里的 `description` 是否写得清楚。AI 是通过这段文字理解工具用途的。
- **Q: 编译报错 `cJSON.h` 找不到？**
  A: 确保在 `.c` 文件顶部 `#include "cJSON.h"`。
- **Q: 如何调试？**
  A: 在 C 代码里使用 `ESP_LOGI(TAG, "我的调试信息: %d", val);`，烧录后通过串口查看输出。

---
祝你在 MimiClaw 的二次开发中玩得开心！🚀
