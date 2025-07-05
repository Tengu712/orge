# Architecture

## Modules

```mermaid
graph TD
    orge -.-> graphics
    orge -.-> rendering
    graphics -.-> rendering
    graphics -.-> mesh
    rendering ==> swapchain
    rendering ==> pipeline
    swapchain --> window
    pipeline --> swapchain
```

- 太線矢印: 依存
- 細線矢印: 参照
- 点線矢印: 転送
