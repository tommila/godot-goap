# Godot GOAP

## Introduction
A simple GOAP or Goal Oriented Action Planning c++ interface for Godot 4.x game engine that uses the [Abraham T. Stolk](https://github.com/stolk/GPGOAP) GOAPimplementation.

## Building
Add this to your godot modules folder or reference it with scons **custom_modules** parameter.

## Example 
	func goap_football():
		# Create GoapNode object
		var goap_node := GoapNode.new()
		
		# Create GoapAction resources for getting ball, going near goal and kicking ball
		var action_get_ball := GoapAction.new()
		action_get_ball.resource_name = "ActionGetBall"
		
		action_get_ball.add_precondition_data({
			"HasBall": false        
		})
		action_get_ball.add_effect_data({
			"HasBall": true
		})
		
		var action_goto_goal := GoapAction.new()
		action_goto_goal.resource_name = "ActionGotoGoal"
		action_goto_goal.add_precondition_data({
			"NearGoal": false        
		})
		action_goto_goal.add_effect_data({
			"NearGoal": true
		})
		
		var action_kick_ball := GoapAction.new()
		action_goto_goal.resource_name = "ActionKickBall"
		action_kick_ball.add_precondition_data({
			"NearGoal": true,
			"HasBall": true,      
		})
		action_kick_ball.add_effect_data({
			"ScoreGoal": true,
			"HasBall": false
		})    
		
		# Add GoapAction resources to GoapNode
		goap_node.add_action_resource(action_get_ball)
		goap_node.add_action_resource(action_goto_goal)
		goap_node.add_action_resource(action_kick_ball)
		
		# Define current world state and the goal state.
		# In this case the world state would represent a player that does not have 
		# a ball and isn't near the goal.
		# For the goal state we would want our player to get the ball and then
		# try tha score a goal. 
		goap_node.add_world_state({
			"HasBall": false,
			"NearGoal": false,
			"ScoreGoal": false
		})
		
		goap_node.add_goal_state({
			"ScoreGoal": true
		})    
		
		# Create the GOAP plan
		goap_node.create_plan(false)
		
		# Pop next action from the plan stack until
		# there are no more actions left and the goal has been achieved
		while(true):
			var goap_action = goap_node.pop_next_action()
			if goap_action:
				print(goap_action.resource_name)
				## Do something with action
			else:
				break

