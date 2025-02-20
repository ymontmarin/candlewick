#include "Core.h"
#include "Tags.h"
#include <functional>

namespace candlewick {

class GuiSystem {
  Renderer const *_renderer;

public:
  using GuiBehavior = std::function<void(const Renderer &)>;

  GuiSystem(NoInitT, GuiBehavior behav)
      : _renderer(nullptr), _callback(behav) {}
  GuiSystem(const Renderer &renderer, GuiBehavior behav);

  bool init(const Renderer &renderer);
  void render(CommandBuffer &cmdBuf);
  void release();

  GuiBehavior _callback;

private:
  bool _initialized = false;
};

} // namespace candlewick
