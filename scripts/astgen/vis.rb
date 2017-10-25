##########################################################################
# 
# FormaB - the bootstrap Forma compiler (vis.rb)
# Copyright (C) 2017 Ryan Schroeder, Colin Unger
# 
# FormaB is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# 
# FormaB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public License
# along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
# 
##########################################################################

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
