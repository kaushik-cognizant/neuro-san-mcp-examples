package com.example.mcpweatherserver;

import org.springframework.ai.tool.annotation.Tool;
import org.springframework.ai.tool.annotation.ToolParam;
import org.springframework.stereotype.Service;

@Service
public class UtilityService {

    @Tool(description = "Calculate BMI given weight in kg and height in meters")
    public double calculateBmi(
        @ToolParam(description = "Weight in kilograms") double weight,
        @ToolParam(description = "Height in meters") double height) {
        return weight / (height * height);
    }

    @Tool(description = "Get a personalized greeting")
    public String getGreeting(
        @ToolParam(description = "The name to greet") String name) {
        return "Hello, " + name + "!";
    }
}