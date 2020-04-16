luabt = require('luabt')

function init()
   --[[ check ARGoS version ]]
   if type(robot.random.bernoulli()) ~= "boolean" then error("Please upgrade and reinstall ARGoS") end
   --[[ constants ]]--
   constants = {
      target_density = 0.2,
      confidence_coefficient = 0.05,
      ttl = 3,
   }
   --[[ accumulator ]]--
   accumulator = {
      current_index = 1,
      length = 250,
      samples = {},
      sum = 0,
   }
   --[[ database ]]--
   database = {}
   --[[ function for determining whether an obstacle exists ]]-- 
   robot.obstacle_detected = function()
      local rangefinders = {
         robot.rangefinders[7].reading,
         robot.rangefinders[8].reading,
         robot.rangefinders[1].reading,
         robot.rangefinders[2].reading,
      }
      for index, reading in ipairs(rangefinders) do
         if reading < 0.1 then
            return true
         end
      end
      return false
   end
   --[[ behavior tree for implementing obstacle avoidance ]]--
   robot.avoid_obstacles = luabt.create({
      type = "selector",
      children = {{
         type = "sequence",
         children = {
            function()
               return false, robot.obstacle_detected()
            end,
            function()
               robot.differential_drive.set_target_velocity(0.05, 0.05)
               return true
            end,
         }},
         function()
            robot.differential_drive.set_target_velocity(0.025, -0.025)
            return true
         end,
      }
   })  
end

function step()
   --[[ tick obstacle avoidance behavior tree ]]--
   robot.avoid_obstacles()
   --[[ track the ratio of shaded to non-shaded cells ]]--
   if not robot.obstacle_detected() then
      if robot.ground.center.reading < 0.75 then
         accumulator.samples[accumulator.current_index] = 1
      else
         accumulator.samples[accumulator.current_index] = 0
      end
      -- recalculate the sum
      accumulator.sum = 0
      for index, value in ipairs(accumulator.samples) do
         accumulator.sum = accumulator.sum + value
      end
      -- update index
      if accumulator.current_index < accumulator.length then
         accumulator.current_index = accumulator.current_index + 1
      else
         accumulator.current_index = 1
      end
   end
   --[[ transmit the state of our accumulator and our neighbour's accumulators ]]--
   local message = {
      [robot.id] = {
         sum = accumulator.sum,
         samples = #accumulator.samples,
         ttl = constants.ttl,
      }
   }
   for other_robot, other_robot_data in pairs(database) do
      if other_robot_data.ttl > 0 then
         other_robot_data.ttl = other_robot_data.ttl - 1
         message[other_robot] = {
            sum = other_robot_data.sum,
            samples = other_robot_data.samples,
            ttl = other_robot_data.ttl,
         }
      end
   end
   robot.wifi.tx_data(message)
   --[[ update our database with information from recieved messages ]]--
   for index, message in ipairs(robot.wifi.rx_data) do
      for other_robot, other_robot_data in pairs(message) do
         if other_robot ~= robot.id then
            if database[other_robot] ~= nil then
               if other_robot_data.ttl > database[other_robot].ttl then
                  -- update our entry
                  database[other_robot].sum = other_robot_data.sum
                  database[other_robot].samples = other_robot_data.samples
                  database[other_robot].ttl = other_robot_data.ttl
               end
            else
               -- create a new entry
               database[other_robot] = {
                  sum = other_robot_data.sum,
                  samples = other_robot_data.samples,
                  ttl = other_robot_data.ttl,
               }
            end
         end
      end
   end
   --[[ estimate the tile density ]]--
   local total_samples = #accumulator.samples
   local total_sum = accumulator.sum
   for other_robot, other_robot_data in pairs(database) do
      if other_robot_data.ttl > 0 then
         total_samples = total_samples + other_robot_data.samples
         total_sum = total_sum + other_robot_data.sum
      end
   end
   if total_samples > 5 then
      local estimate = total_sum / total_samples
      --[[ determine whether or not we should switch to foraging ]]--
      if constants.target_density > estimate then
         local probablity = math.min(1.0, (constants.target_density - estimate))
         if robot.random.bernoulli(probablity) then
            robot.debug.set_task("foraging")
         end
      end
      estimate = string.format("%.3f", estimate):gsub(",",".")
      robot.debug.set_estimate(estimate)
   end
end

function reset() end
function destroy() end
