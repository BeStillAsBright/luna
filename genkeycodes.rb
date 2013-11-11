File.open("keycodes.txt", "w") do |f|
  # regular number keys
  (0..9).each do |c|
    f.puts "case SDLK_#{c}:"
    f.puts "\tlua_pushliteral(L,\"#{c}\");"
    f.puts "\tbreak;"
  end

  # regular letters
  ('a'..'z').each do |c|
    f.puts "case SDLK_#{c}:"
    f.puts "\tlua_pushliteral(L,\"#{c}\");"
    f.puts "\tbreak;"
  end

  # F keys
  (1..24).each do |c|
    f.puts "case SDLK_F#{c}:"
    f.puts "\tlua_pushliteral(L,\"f#{c}\");"
    f.puts "\tbreak;"
  end

  # keypad numbers
  (0..9).each do |c|
    f.puts "case SDLK_KP_#{c}:"
    f.puts "\tlua_pushliteral(L,\"kp#{c}\");"
    f.puts "\tbreak;"
  end

end
