import SwiftUI

/// Editor for one step's trig and its parameter locks ("p-locks").
struct ParamLockPanel: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project
    let trackIndex: Int
    let stepIndex: Int

    private var stepBinding: Binding<Step> {
        appState.stepBinding(track: trackIndex, index: stepIndex)
    }

    var body: some View {
        let step = stepBinding.wrappedValue

        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text("Step \(stepIndex + 1)").font(.subheadline).bold()

                Toggle("Trig", isOn: Binding(
                    get: { step.trig != nil },
                    set: { enabled in
                        var s = stepBinding.wrappedValue
                        s.trig = enabled ? (s.trig ?? Step.Trig()) : nil
                        stepBinding.wrappedValue = s
                    }
                ))

                if step.trig != nil {
                    Picker("", selection: Binding(
                        get: { stepBinding.wrappedValue.trig?.type ?? .note },
                        set: { newType in
                            var s = stepBinding.wrappedValue
                            s.trig?.type = newType
                            stepBinding.wrappedValue = s
                        }
                    )) {
                        Text("Note").tag(TrigType.note)
                        Text("Lock").tag(TrigType.lock)
                    }
                    .frame(width: 120)
                    .labelsHidden()
                }

                Spacer()
            }

            if step.trig != nil {
                HStack {
                    Text("Vel").font(.caption2).frame(width: 34, alignment: .leading)
                    Slider(value: Binding(
                        get: { stepBinding.wrappedValue.trig?.velocity ?? 1 },
                        set: { v in var s = stepBinding.wrappedValue; s.trig?.velocity = v; stepBinding.wrappedValue = s }
                    ))
                    Text("Timing").font(.caption2).frame(width: 44, alignment: .leading)
                    Slider(value: Binding(
                        get: { stepBinding.wrappedValue.trig?.microTiming ?? 0 },
                        set: { v in var s = stepBinding.wrappedValue; s.trig?.microTiming = v; stepBinding.wrappedValue = s }
                    ), in: -0.5...0.5)
                }

                ScrollView {
                    VStack(spacing: 4) {
                        ForEach(ParameterID.allCases) { parameter in
                            LockRow(track: project.tracks[trackIndex], trackIndex: trackIndex, stepIndex: stepIndex, parameter: parameter)
                        }
                    }
                }
                .frame(maxHeight: 150)
            } else {
                Text("No trig on this step. Turn on Trig to sequence it, or add a Lock trig to change parameters without retriggering the sample.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .padding(8)
        .background(Color.gray.opacity(0.06))
        .cornerRadius(6)
    }
}

private struct LockRow: View {
    @EnvironmentObject var appState: AppState
    @ObservedObject var track: Track
    let trackIndex: Int
    let stepIndex: Int
    let parameter: ParameterID

    private var stepBinding: Binding<Step> {
        appState.stepBinding(track: trackIndex, index: stepIndex)
    }

    private var baseValue: Double {
        track.parameters[parameter]
    }

    var body: some View {
        let locked = stepBinding.wrappedValue.trig?.locks[parameter] != nil

        HStack {
            Toggle(isOn: Binding(
                get: { locked },
                set: { on in
                    var s = stepBinding.wrappedValue
                    if on {
                        s.trig?.locks[parameter] = baseValue
                    } else {
                        s.trig?.locks.removeValue(forKey: parameter)
                    }
                    stepBinding.wrappedValue = s
                }
            )) {
                Text(parameter.displayName).font(.caption2).frame(width: 90, alignment: .leading)
            }
            .toggleStyle(.checkbox)

            Slider(value: Binding(
                get: { stepBinding.wrappedValue.trig?.locks[parameter] ?? baseValue },
                set: { v in
                    var s = stepBinding.wrappedValue
                    guard s.trig?.locks[parameter] != nil else { return }
                    s.trig?.locks[parameter] = v
                    stepBinding.wrappedValue = s
                }
            ))
            .disabled(!locked)
        }
    }
}
