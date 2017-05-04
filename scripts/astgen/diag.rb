class Diagnostics
  def initialize(pos:[], on_error:, quiet:)
    @pos = pos
    @on_error = on_error
    @quiet = quiet
  end

  def push(str) @pos.push(str); self end
  def pop; @pos.pop(); self end

  def pos(str)
    push(str)

    begin
      return yield
    ensure
      pop()
    end
  end

  def trace(lvl, str)
    $stderr << "[#{@pos.join(" :: ")}] #{lvl}: #{str}\n"
    str
  end

  def good(lvl, str)
    trace(lvl, str) unless @quiet.call
    str
  end

  def bad(lvl, str)
    trace(lvl, str)
    @on_error.call
    str
  end

  def debug(str) good("debug", str) end
  def info(str) good("info", str) end
  def warn(str) good("warning", str) end
  def error(str) bad("error", str) end
  def fatal(str) bad("fatal", str) end
end