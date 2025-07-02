# Architecture

## Modules

```mermaid
graph TD
    oroge -.-> graphics
    graphics -.-> rendering
    rendering ==> swapchain
    rendering ==> pipeline
    swapchain --> window
```

- 太線矢印: 依存
- 細線矢印: 参照
- 点線矢印: 転送
