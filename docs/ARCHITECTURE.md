# Architecture

## Modules

### graphics

```mermaid
graph TD
    rendering ==> swapchain
    rendering ==> pipeline
    rendering ==> mesh
    swapchain --> window
    pipeline --> buffer
```
