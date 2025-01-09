#include "Scene.h"

namespace candlewick {

class GuiSystem {
public:
  using GuiBehavior = std::function<void(Renderer &)>;

  GuiSystem(GuiBehavior behav) : callback_(behav) {}

  bool init(const Renderer &renderer);
  void render(Renderer &renderer);
  void release();

  GuiBehavior callback_;
};

static_assert(Scene<GuiSystem>);

} // namespace candlewick
