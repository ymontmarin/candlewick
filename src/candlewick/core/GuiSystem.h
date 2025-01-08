#include "Scene.h"

namespace candlewick {

class GuiSystem {
public:
  using GuiBehavior = std::function<void(Renderer &)>;

  GuiSystem(GuiBehavior behav) : callback_(behav) {}

  void render(Renderer &render);

  GuiBehavior callback_;
};

static_assert(Scene<GuiSystem>);

} // namespace candlewick
