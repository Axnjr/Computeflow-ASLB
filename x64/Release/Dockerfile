# Use a Windows Server Core image with .NET runtime or Visual C++ Redistributable if needed
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Set the working directory
WORKDIR /app

# Copy the compiled executable and any necessary files into the container
COPY /x64/Release/* .

# Expose the port that your application runs on
EXPOSE 4000

# Define the command to run your application
CMD ["aslb.exe"]