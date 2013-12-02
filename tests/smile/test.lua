local luna = require 'luna'

local texlib = {}

local Smile = {}
function Smile:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	
	o.image = texlib.smile
	o.x = 0
	o.y = 0
	return o
end
function Smile:draw(win)
	win:draw(self.image, self.x, self.y)
end
function Smile:move(dx,dy)
	self.x = self.x + dx
	self.y = self.y + dy
end

function main()
	luna.init()
	win = luna.window.new(640,480,false)

	texlib.smile = luna.texture.new(win, 'smile.png')
	texlib.grass = luna.texture.new(win, 'grass.png')

	my_smile = Smile:new()
	-- other_smile = Smile:new()
	win:show()
	while true do
		etype, event = luna.event.poll()
		if etype == 'key_down' then
			key = event.key
			if key == 'w' then
				my_smile:move(0,-5)
			elseif key == 's' then
				my_smile:move(0,5)
			elseif key == 'a' then
				my_smile:move(-5,0)
			elseif key == 'd' then
				my_smile:move(5,0)
			elseif key == 'escape' then
				break
			end
		end
		-- draw
		win:clear()
		-- draw bg
		win:draw(texlib.grass,0,0)
		my_smile:draw(win)
		win:paint()
		luna.delay(17)
	end
	luna.quit()
end

main()
