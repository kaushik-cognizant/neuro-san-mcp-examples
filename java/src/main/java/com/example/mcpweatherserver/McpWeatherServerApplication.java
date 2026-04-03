package com.example.mcpweatherserver;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

import org.springframework.ai.tool.ToolCallbackProvider;
import org.springframework.ai.tool.method.MethodToolCallbackProvider;
import org.springframework.context.annotation.Bean;


@SpringBootApplication
public class McpWeatherServerApplication {

    public static void main(String[] args) {
        SpringApplication.run(McpWeatherServerApplication.class, args);
    }


	@Bean
	public ToolCallbackProvider weatherTools(WeatherService weatherService, UtilityService utilityService) {
		return MethodToolCallbackProvider.builder()
				.toolObjects(weatherService, utilityService)
				.build();
	}
}