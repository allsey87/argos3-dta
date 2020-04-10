luabt = require('luabt')

function init()      
   --[[ table of fixed robot parameters ]]--
   robot.constants = {
      m = 20,
      m2 = 100,
      opt_dens = 0.3,
   }
   --[[ table of variable robot parameters ]]--
   robot.variables = {
      --[[ create an entry in the robot table to store
         the readings from the ground sensors ]]--
      accumulator = 0.0,
      --[[ create an entry in the robot table to store
         the robot's estimates ]]--
      estimate = 0.0,
      prev_iter_estimate = 0.0,
      deviation = 0.0,
      --[[ create an entry in the robot table to store
         the robot's state ]]--
      state = "initial_estimation",
      --[[ create entries in the robot table to store
          the robot's time counters ]]--
      curr_expl_time = 0,   
      tot_init_expl_time = robot.random.poisson(robot.constants.m2),
      tot_expl_time = robot.random.poisson(robot.constants.m),
      prev_tot_expl_time = tot_expl_time,
      --[[ create an entry in the robot table to store
         the robot's degree (i.e. number of neighbors 
         the robot is communicating with) ]]--
      degree = 0,
   }
   --[[ create an entry in the robot table to store
        the obstacle avoidance behavior tree ]]--
   robot.avoid_obstacles = luabt.create({
      type = "selector",
      children = {{
         type = "sequence",
         children = {
            function()
               local rangefinders = {
                  far_left  = robot.rangefinders[7].reading,
                  left      = robot.rangefinders[8].reading,
                  right     = robot.rangefinders[1].reading,
                  far_right = robot.rangefinders[2].reading,
               }
               for rangefinder, reading in pairs(rangefinders) do
                  if reading < 0.1 then
                     return false, true
                  end
               end
               return false, false
            end,
            function()
               robot.differential_drive.set_target_velocity(0.05, 0.05)
               return true
            end,
         }},
         function()
            robot.differential_drive.set_target_velocity(0.05, -0.05)
            return true
         end,
      }
   })  
   --[[ create an entry in the robot table to store
        the decision-making process ]]--
   robot.listen = function()
      -- local variables to calculate the average over the neighborhood
      local average_est = 0.0
      robot.variables.degree = 0
      for index, message in ipairs(robot.wifi.rx_data) do
         average_est = average_est + message.est
         robot.variables.degree = robot.variables.degree + 1.0
      end
      if robot.variables.degree > 0 then
         average_est = average_est/robot.variables.degree 
         robot.variables.estimate = robot.variables.estimate + 0.5 * (average_est - robot.variables.estimate)
      end
   end  
end


function step()
   -- tick obstacle avoidance behavior tree
   robot.avoid_obstacles()
   -- sets the current robot task, i.e. foraging, exploring or initial_exploration
   robot.debug.set_task(robot.variables.state)
   -- put the robot id and the value of the ground accumlator in a table
   if robot.variables.state == "exploring" then
      -- the robot is exploring the arena/cache in search for shaded tiles/building blocks
      -- decrement the number of time steps a robot is exploring
      robot.variables.curr_expl_time = robot.variables.curr_expl_time + 1.0
      -- count the number of time steps a robot is on a shaded tile
      -- robot.variables.accumulator = 0
      if robot.ground.center.reading < 0.75 then
        robot.variables.accumulator = robot.variables.accumulator + 1.0
      end
      -- the robot averages its accumulator measurements over robot.variables.tot_diss_time+1 seconds
      if robot.variables.curr_expl_time >= robot.variables.tot_expl_time+1 then
        -- update the robot's extimate and deviation from the desired density opt_dens
        -- robot.variables.estimate = (robot.variables.estimate * robot.variables.prev_tot_expl_time + robot.variables.accumulator) / (1.0*robot.variables.curr_expl_time)
        robot.variables.estimate = (1.0 * robot.variables.accumulator) / (1.0 * robot.variables.curr_expl_time)
        robot.variables.deviation = (robot.variables.estimate - robot.constants.opt_dens)
        robot.variables.prev_iter_estimate = robot.variables.estimate
        robot.variables.prev_tot_expl_time = robot.variables.curr_expl_time
        robot.variables.curr_expl_time = 0
        robot.variables.accumulator = 0
        robot.variables.tot_expl_time = robot.random.poisson(robot.constants.m)
        -- switch probabilistically to foraging, iff (robot.variables.estimate - robot.constants.opt_dens) < 0
        if robot.random.uniform() < robot.constants.opt_dens and robot.variables.deviation < 0 then
          robot.variables.state = "foraging"
        end
     end
     --~ put the robot id and its estimate in a table
      local data = {
         id = robot.id,
         est = robot.variables.estimate,
      }
      -- broadcast that table over wifi
      robot.wifi.tx_data(data)
      -- now listen to others
      robot.listen()
   -- perform the initial_exploration if you just arrived from foraging
   elseif robot.variables.state == "initial_estimation" then
      -- decrement the number of time steps a robot is perorming initial_estimation
      robot.variables.curr_expl_time = robot.variables.curr_expl_time + 1.0
      
      -- count the number of time steps a robot is on a shaded tile
      if robot.ground.center.reading < 0.75 then
        robot.variables.accumulator = robot.variables.accumulator + 1.0
      end
      -- has the robot finished initial exploration?
      if robot.variables.curr_expl_time >= robot.variables.tot_init_expl_time then
         -- Prepare the robot to switch to exploring
         robot.variables.state = "exploring"
         robot.variables.estimate = robot.variables.accumulator / (1.0*robot.variables.curr_expl_time)
         robot.variables.prev_iter_estimate = robot.variables.estimate
         robot.variables.accumulator = 0
         robot.variables.prev_tot_expl_time = robot.variables.curr_expl_time
         robot.variables.curr_expl_time = 0
         robot.variables.tot_expl_time = robot.random.poisson(robot.constants.m)
      end
   end
   -- send the current estimate, deviation and degree to the loop functions
   local estimate = string.format("%3.5f", robot.variables.estimate)
   estimate = estimate:gsub(",", ".")
   robot.debug.set_estimate(estimate)
   local degree = string.format("%3.5f", robot.variables.degree)
   degree = degree:gsub(",", ".")
   robot.debug.set_degree(degree)
end

function reset() end
function destroy() end
