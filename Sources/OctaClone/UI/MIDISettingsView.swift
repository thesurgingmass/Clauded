import SwiftUI

struct MIDISettingsView: View {
    @EnvironmentObject var project: Project
    @EnvironmentObject var midiManager: MIDIManager
    @State private var learnTrackIndex = 0
    @State private var learnParameter: ParameterID = .volume

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("MIDI Settings").font(.title2).bold()

            GroupBox("Devices") {
                VStack(alignment: .leading, spacing: 10) {
                    devicePicker(
                        title: "Input",
                        selection: project.midi.inputEndpointName,
                        options: midiManager.availableInputs,
                        onChange: { name in
                            project.midi.inputEndpointName = name
                            midiManager.connectInput(named: name)
                        }
                    )
                    devicePicker(
                        title: "Output",
                        selection: project.midi.outputEndpointName,
                        options: midiManager.availableOutputs,
                        onChange: { name in
                            project.midi.outputEndpointName = name
                            midiManager.connectOutput(named: name)
                        }
                    )
                    Button("Refresh Device List") {
                        midiManager.refreshEndpoints()
                    }
                }
                .padding(6)
            }

            GroupBox("Clock & Transport") {
                VStack(alignment: .leading, spacing: 10) {
                    Picker("Clock Source", selection: Binding(
                        get: { project.midi.clockSource },
                        set: { project.midi.clockSource = $0 }
                    )) {
                        ForEach(ClockSource.allCases) { source in
                            Text(source.displayName).tag(source)
                        }
                    }
                    Toggle("Send MIDI Clock (when playing internally)", isOn: Binding(
                        get: { project.midi.sendClock },
                        set: { project.midi.sendClock = $0 }
                    ))
                    Toggle("Incoming Program Change selects pattern", isOn: Binding(
                        get: { project.midi.programChangeSelectsPattern },
                        set: { project.midi.programChangeSelectsPattern = $0 }
                    ))
                    Text("Each track also has its own MIDI channel + note (set on its track strip) that triggers it directly, independent of the clock source above.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .padding(6)
            }

            GroupBox("CC → Parameter Mappings") {
                VStack(alignment: .leading, spacing: 10) {
                    HStack {
                        Picker("Track", selection: $learnTrackIndex) {
                            ForEach(Array(project.tracks.enumerated()), id: \.offset) { index, track in
                                Text(track.name).tag(index)
                            }
                        }
                        .frame(width: 160)

                        Picker("Parameter", selection: $learnParameter) {
                            ForEach(ParameterID.allCases) { parameter in
                                Text(parameter.displayName).tag(parameter)
                            }
                        }
                        .frame(width: 200)

                        if midiManager.isLearning {
                            Button("Cancel") { midiManager.cancelLearning() }
                            Text("Move a knob on your controller…").font(.caption).foregroundStyle(.orange)
                        } else {
                            Button("Learn") {
                                midiManager.beginLearning(trackIndex: learnTrackIndex, parameter: learnParameter)
                            }
                        }
                    }

                    Divider()

                    if project.midi.ccMappings.isEmpty {
                        Text("No mappings yet.").font(.caption).foregroundStyle(.secondary)
                    } else {
                        ForEach(project.midi.ccMappings) { mapping in
                            HStack {
                                Text("Ch \(mapping.channel + 1)  CC\(mapping.controller)")
                                    .font(.caption.monospaced())
                                Image(systemName: "arrow.right")
                                Text("\(project.tracks[safe: mapping.trackIndex]?.name ?? "?") · \(mapping.parameter.displayName)")
                                    .font(.caption)
                                Spacer()
                                Button(role: .destructive) {
                                    project.midi.ccMappings.removeAll { $0.id == mapping.id }
                                } label: {
                                    Image(systemName: "trash")
                                }
                                .buttonStyle(.plain)
                            }
                        }
                    }
                }
                .padding(6)
            }

            Spacer()
        }
        .padding(20)
        .onAppear { midiManager.refreshEndpoints() }
    }

    private func devicePicker(title: String, selection: String?, options: [MIDIEndpointInfo], onChange: @escaping (String?) -> Void) -> some View {
        Picker(title, selection: Binding(get: { selection }, set: onChange)) {
            Text("None").tag(String?.none)
            ForEach(options) { endpoint in
                Text(endpoint.name).tag(String?.some(endpoint.name))
            }
        }
    }
}
