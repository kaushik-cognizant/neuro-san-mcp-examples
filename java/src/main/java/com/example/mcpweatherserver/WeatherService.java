package com.example.mcpweatherserver;

import org.springframework.ai.tool.annotation.Tool;
import org.springframework.ai.tool.annotation.ToolParam;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestClient;

import java.time.LocalDateTime;

@Service
public class WeatherService {

	public record WeatherResponse(Current current) {
		public record Current(LocalDateTime time, int interval, double temperature_2m) {}
	}

	@Tool(description = "Get the temperature (in celsius) for a specific location")
	public WeatherResponse getTemperature(
      @ToolParam(description = "The location latitude") double latitude,
      @ToolParam(description = "The location longitude") double longitude) {

		return RestClient.create()
				.get()
				.uri("https://api.open-meteo.com/v1/forecast?latitude={latitude}&longitude={longitude}&current=temperature_2m",
						latitude, longitude)
				.retrieve()
				.body(WeatherResponse.class);
	}
}