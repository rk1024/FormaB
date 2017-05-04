require_relative 'linewriter'

class Vis
  def initialize(l, vis)
    @l = l.curr
    @curr = vis
  end

  def <<(vis)
    if @curr != vis
      @l.sep
      @l << "#{vis.to_s}:"

      @curr = vis
    end

    self
  end
end
