luabt = require('luabt')

function init()
   -- ground accumator
   ground_accumulator = 0
   -- obstacle avoidance behavior
   behavior = luabt.create({
      type = "selector",
      children = {{
         type = "sequence",
         children = {
            function() return false, obstacle_detected end,
            function() robot.differential_drive.set_target_velocity(0.05, 0.05) return true end,
         }},
         function() robot.differential_drive.set_target_velocity(0.05, -0.05) return true end,
      }
   })
end

function step()
   -- process obstacles
   obstacle_detected = false
   local obstacles = {
      left = robot.rangefinders[2].reading,
      front = robot.rangefinders[1].reading,
      right = robot.rangefinders[12].reading,
   }
   for obstacle, distance in pairs(obstacles) do
      if distance < 0.1 then
         obstacle_detected = true
      end
   end
   -- tick obstacle avoidance behavior tree
   behavior()
   -- tell the loop functions we want to switch to foraging
   if robot.ground.center.reading < 0.75 then
      ground_accumulator = ground_accumulator + 1
      if ground_accumulator > 25 then
         robot.debug.loop_functions("foraging");
      end
   end
   -- send robot id and value in the ground accumlator
   robot.wifi.tx_data({robot.id, ground_accumulator})
end

function reset() end
function destroy() end
