#include "Core.h"
#include "Tags.h"
#include <functional>

namespace candlewick {

class GuiSystem {
public:
  using GuiBehavior = std::function<void(Renderer &)>;

  GuiSystem(NoInitT, GuiBehavior behav) : callback_(behav) {}
  GuiSystem(const Renderer &renderer, GuiBehavior behav);

  bool init(const Renderer &renderer);
  void render(Renderer &renderer);
  void release();

  GuiBehavior callback_;

private:
  bool _initialized = false;
};

} // namespace candlewick
