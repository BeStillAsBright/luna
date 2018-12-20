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
		if luna.keyboard.key_down('return') then
			break
		end
		if etype == 'key_down' then
			key = event.key
			if key == 'w' then
				local x = 0
				local y = -5
				if luna.keyboard.mod_down('shift') then
					x = -x
					y = -y
				end
				my_smile:move(x,y)
			elseif key == 's' then
				local x = 0
				local y = 5
				if luna.keyboard.mod_down('shift') then
					x = -x
					y = -y
				end
				my_smile:move(x,y)
			elseif key == 'a' then
				local x = -5
				local y = 0
				if luna.keyboard.mod_down('shift') then
					x = -x
					y = -y
				end
				my_smile:move(x,y)
			elseif key == 'd' then
				local x = 5
				local y = 0
				if luna.keyboard.mod_down('shift') then
					x = -x
					y = -y
				end
				my_smile:move(x,y)
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
