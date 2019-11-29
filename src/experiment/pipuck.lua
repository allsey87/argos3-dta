luabt = require('luabt')

function init()
   behavior = luabt.create({
      type = "selector",
      children = {{
         type = "sequence",
         children = {
            function() return false, closest_obstacle == "left" or closest_obstacle == "front" end,
            function() robot.differential_drive.set_target_velocity(0.05, 0.05) return true end,
         }}, {
         type = "sequence",
         children = {
            function() return false, closest_obstacle == "right" end,
            function() robot.differential_drive.set_target_velocity(-0.05, -0.05) return true end,

         }},
         function() robot.differential_drive.set_target_velocity(0.05, -0.05) return true end,
      }
   })
end

function step()
   -- process obstacles
   closest_obstacle = nil
   local obstacles = {
      left = robot.rangefinders[8].reading,
      front = robot.rangefinders[1].reading,
      right = robot.rangefinders[2].reading,
   }
   for obstacle, distance in pairs(obstacles) do
      if distance < 0.1 then
         if closest_obstacle == nil or distance < obstacles[closest_obstacle] then
            closest_obstacle = obstacle
         end
      end
   end
   -- tick behavior tree
   behavior()
end

function reset() end
function destroy() end
