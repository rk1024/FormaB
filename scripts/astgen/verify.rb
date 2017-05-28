module Kernel
  def verify(cond)
    yield unless cond
    cond
  end
end